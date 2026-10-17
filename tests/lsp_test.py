#!/usr/bin/env python3
# End-to-end tests for icl-lsp.
#
# The message sequence mirrors what coc.nvim actually sends
# (initialize -> initialized -> didOpen -> formatting -> shutdown -> exit),
# because "speaks LSP" and "works with coc.nvim" are not the same thing: a
# server can be spec-compliant and still fail to start in a given client over
# details like the sync kind or an unexpected notification.

import argparse
import json
import os
import queue
import subprocess
import sys
import threading

PASS = 0
FAIL = 0


def check(name, ok, detail=''):
    global PASS, FAIL
    if ok:
        PASS += 1
        print('  [PASS] %s' % name)
    else:
        FAIL += 1
        print('  [FAIL] %s  %s' % (name, detail))


class Client:
    """Speaks the LSP base protocol over a subprocess's stdin/stdout."""

    def __init__(self, binary):
        self.proc = subprocess.Popen(
            [binary],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
        self._queue = queue.Queue()
        self._reader = threading.Thread(target=self._read_loop, daemon=True)
        self._reader.start()
        self.next_id = 1
        self._stray = []

    def _read_loop(self):
        """Parses Content-Length framing off the raw stream."""
        stream = self.proc.stdout
        try:
            while True:
                header = b''
                while not header.endswith(b'\r\n\r\n'):
                    byte = stream.read(1)
                    if not byte:
                        return
                    header += byte
                length = None
                for line in header.decode('utf-8', 'replace').split('\r\n'):
                    if line.lower().startswith('content-length:'):
                        length = int(line.split(':', 1)[1].strip())
                if length is None:
                    return
                body = stream.read(length)
                if body is None or len(body) != length:
                    return
                try:
                    message = json.loads(body.decode('utf-8'))
                except ValueError:
                    self._stray.append(body)
                    continue
                self._queue.put(message)
        except Exception:  # noqa: BLE001 - the reader must never wedge a test
            return

    def send(self, message):
        body = json.dumps(message).encode('utf-8')
        self.proc.stdin.write(
            b'Content-Length: %d\r\n\r\n' % len(body) + body)
        self.proc.stdin.flush()

    def request(self, method, params=None):
        """Sends a request and waits for the response with the matching id."""
        request_id = self.next_id
        self.next_id += 1
        self.send({'jsonrpc': '2.0', 'id': request_id, 'method': method,
                   'params': params if params is not None else {}})
        return self.wait(lambda m: m.get('id') == request_id,
                         'response to %s' % method)

    def notify(self, method, params=None):
        self.send({'jsonrpc': '2.0', 'method': method,
                   'params': params if params is not None else {}})

    def wait(self, predicate, what, timeout=10.0):
        """Waits for the first queued message satisfying |predicate|."""
        pending = []
        try:
            while True:
                message = self._queue.get(timeout=timeout)
                if predicate(message):
                    for skipped in pending:
                        self._queue.put(skipped)
                    return message
                pending.append(message)
        except queue.Empty:
            for skipped in pending:
                self._queue.put(skipped)
            check('timed out waiting for %s' % what, False)
            return None

    def drain(self, timeout=0.4):
        """Collects everything already queued."""
        out = []
        try:
            while True:
                out.append(self._queue.get(timeout=timeout))
        except queue.Empty:
            pass
        return out

    def stop(self):
        try:
            self.proc.stdin.close()
        except Exception:  # noqa: BLE001
            pass
        try:
            self.proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self.proc.kill()


def coc_initialize(client, root):
    """The initialize request as coc.nvim sends it."""
    return client.request('initialize', {
        'processId': os.getpid(),
        'rootUri': 'file://' + root,
        'rootPath': root,
        'capabilities': {
            'workspace': {'didChangeConfiguration': {'dynamicRegistration':
                                                     True}},
            'textDocument': {
                'synchronization': {'dynamicRegistration': True},
                'formatting': {'dynamicRegistration': True},
            },
        },
        'trace': 'off',
        'workspaceFolders': [{'uri': 'file://' + root, 'name': os.path.basename(root)}],
    })


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', default='build/icl-lsp')
    parser.add_argument('--cli', default='build/icl-format',
                        help='icl-format binary, used as the formatting oracle')
    args = parser.parse_args()

    for path in (args.binary, args.cli):
        if not os.path.exists(path):
            print('missing binary: %s' % path)
            return 1

    print('=' * 72)
    print('A. Lifecycle (as coc.nvim drives it)')
    print('=' * 72)

    client = Client(args.binary)
    response = coc_initialize(client, '/tmp')
    ok = response is not None and 'result' in response
    check('initialize -> result', ok, str(response)[:120])
    caps = response.get('result', {}).get('capabilities', {}) if response else {}
    check('advertises formatting',
          caps.get('documentFormattingProvider') is True, str(caps)[:160])
    sync = caps.get('textDocumentSync')
    # Either shape is legal; coc.nvim reads both.
    kind = sync if isinstance(sync, int) else (sync or {}).get('change')
    check('advertises full text sync (kind 1)', kind == 1, 'sync=%r' % sync)

    client.notify('initialized', {})
    # coc.nvim sends these on start; a server that replies to a notification
    # or dies here never shows up as running.
    client.notify('workspace/didChangeConfiguration',
                  {'settings': {'icl': {'indent': 2}}})
    client.notify('$/setTrace', {'value': 'off'})

    print()
    print('=' * 72)
    print('B. Diagnostics')
    print('=' * 72)

    good_uri = 'file:///tmp/good.icl'
    bad_uri = 'file:///tmp/bad.icl'

    client.notify('textDocument/didOpen', {
        'textDocument': {'uri': good_uri, 'languageId': 'icl', 'version': 1,
                         'text': 'Module m {\n  ScanInPort SI;\n}\n'}})
    message = client.wait(
        lambda m: m.get('method') == 'textDocument/publishDiagnostics',
        'diagnostics for a valid file')
    diags = (message or {}).get('params', {}).get('diagnostics', [])
    check('valid file -> no diagnostics', diags == [], str(diags)[:160])

    client.notify('textDocument/didOpen', {
        'textDocument': {'uri': bad_uri, 'languageId': 'icl', 'version': 1,
                         'text': 'Module m {\n  ScanInPort SI\n}\n'}})
    message = client.wait(
        lambda m: m.get('method') == 'textDocument/publishDiagnostics' and
        m.get('params', {}).get('uri') == bad_uri,
        'diagnostics for an invalid file')
    diags = (message or {}).get('params', {}).get('diagnostics', [])
    check('invalid file -> one diagnostic', len(diags) == 1, str(diags)[:200])
    if diags:
        start = diags[0]['range']['start']
        # Bison reports the error at the token it could not use — here the `}`
        # on source line 3, not the incomplete line 2. That is what gcc does
        # too ("expected ';' before '}' token"), and the message names what was
        # expected, so the pair is unambiguous.
        check('diagnostic points at the offending token (source line 3)',
              start['line'] == 2, 'line=%r' % start['line'])
        check('message names the expected token',
              ';' in diags[0].get('message', ''),
              'message=%r' % diags[0].get('message'))
        check('diagnostic has a message', bool(diags[0].get('message')),
              str(diags[0])[:160])
        check('diagnostic severity is Error', diags[0].get('severity') == 1,
              str(diags[0].get('severity')))

    # On a single line there is no lookahead ambiguity: the squiggle must land
    # on line 0.
    client.notify('textDocument/didOpen', {
        'textDocument': {'uri': bad_uri, 'languageId': 'icl', 'version': 2,
                         'text': 'Module { ScanInPort SI; }'}})
    message = client.wait(
        lambda m: m.get('method') == 'textDocument/publishDiagnostics' and
        m.get('params', {}).get('uri') == bad_uri,
        'diagnostics for a single-line error')
    one = (message or {}).get('params', {}).get('diagnostics', [])
    check('single-line error -> diagnostic on line 0',
          bool(one) and one[0]['range']['start']['line'] == 0, str(one)[:200])

    print()
    print('=' * 72)
    print('C. Formatting')
    print('=' * 72)

    source = 'Module m{ScanInPort SI;ScanRegister r[7:0]{ScanInSource SI;}}\n'
    client.notify('textDocument/didOpen', {
        'textDocument': {'uri': good_uri, 'languageId': 'icl', 'version': 2,
                         'text': source}})
    client.drain()  # diagnostics for this version; not what we are testing

    response = client.request('textDocument/formatting', {
        'textDocument': {'uri': good_uri},
        'options': {'tabSize': 2, 'insertSpaces': True}})
    edits = (response or {}).get('result') or []
    check('formatting -> one edit', len(edits) == 1, str(response)[:200])
    if edits:
        new_text = edits[0]['newText']
        expected = subprocess.run(
            [args.cli, '-i'], input=source,
            capture_output=True, text=True).stdout
        check('formatting matches the CLI output', new_text == expected,
              'lsp=%r cli=%r' % (new_text[:80], expected[:80]))
        check('edit covers the whole document',
              edits[0]['range']['start'] == {'line': 0, 'character': 0},
              str(edits[0]['range']))
        check('formatted text is actually re-indented',
              '\n  ScanInPort SI;' in new_text, repr(new_text[:80]))

    # tabSize must reach the formatter: coc.nvim sends the editor's setting.
    response = client.request('textDocument/formatting', {
        'textDocument': {'uri': good_uri},
        'options': {'tabSize': 4, 'insertSpaces': True}})
    edits4 = (response or {}).get('result') or []
    check('honours options.tabSize=4',
          bool(edits4) and '\n    ScanInPort SI;' in edits4[0]['newText'],
          str(edits4)[:200])

    # An absurd tabSize must not become a memory amplifier: the value is
    # applied per indent level, and tabSize=99999 on a six-level file produced
    # a 7.8 MB reply from a 157-byte input. Clamped to 16.
    nested = ('Module m { AccessLink t Of STD_1149_1 { READINSTR '
              '{ ScanPath { a; } } ' + 'Nested { ' * 6 +
              ' Attribute d = 1;' + ' }' * 6 + ' } }')
    client.notify('textDocument/didOpen', {
        'textDocument': {'uri': 'file:///tmp/deep.icl', 'languageId': 'icl',
                         'version': 1, 'text': nested}})
    client.drain()
    response = client.request('textDocument/formatting', {
        'textDocument': {'uri': 'file:///tmp/deep.icl'},
        'options': {'tabSize': 99999, 'insertSpaces': True}})
    deep = ((response or {}).get('result') or [{}])[0].get('newText', '')
    check('absurd tabSize is clamped, not amplified',
          0 < len(deep) < 10000, 'reply was %d bytes' % len(deep))

    # A file with a syntax error must come back untouched rather than mangled.
    client.notify('textDocument/didOpen', {
        'textDocument': {'uri': bad_uri, 'languageId': 'icl', 'version': 3,
                         'text': 'Module m {\n  @ bad\n}\n'}})
    client.drain()
    response = client.request('textDocument/formatting', {
        'textDocument': {'uri': bad_uri},
        'options': {'tabSize': 2, 'insertSpaces': True}})
    check('syntax error -> no edits, not mangled output',
          (response or {}).get('result') == [], str(response)[:200])

    # An incremental change (range, no text) must not blank the document. We
    # advertise full sync, but a client that ignores that must not be able to
    # destroy the buffer.
    client.notify('textDocument/didChange', {
        'textDocument': {'uri': good_uri, 'version': 3},
        'contentChanges': [{'range': {'start': {'line': 0, 'character': 0},
                                      'end': {'line': 0, 'character': 1}},
                            'text': None}]})
    client.drain()
    response = client.request('textDocument/formatting', {
        'textDocument': {'uri': good_uri},
        'options': {'tabSize': 2, 'insertSpaces': True}})
    check('incremental change without text does not blank the document',
          bool((response or {}).get('result')) and
          'ScanInPort SI' in (response or {}).get('result')[0]['newText'],
          str(response)[:200])

    # An unknown document (never opened) must not be invented.
    response = client.request('textDocument/formatting', {
        'textDocument': {'uri': 'file:///tmp/never-opened.icl'},
        'options': {'tabSize': 2, 'insertSpaces': True}})
    check('unknown uri -> no edits', (response or {}).get('result') == [],
          str(response)[:200])

    print()
    print('=' * 72)
    print('D. Robustness')
    print('=' * 72)

    response = client.request('textDocument/hover',
                             {'textDocument': {'uri': good_uri},
                              'position': {'line': 0, 'character': 0}})
    check('unsupported request -> error, not a crash',
          'error' in (response or {}),
          str(response)[:160])
    check('unsupported request -> -32601',
          (response or {}).get('error', {}).get('code') == -32601,
          str(response)[:160])

    # Unknown notifications must be ignored, not answered.
    client.notify('textDocument/didSave', {'textDocument': {'uri': good_uri}})
    still_alive = client.request('textDocument/formatting', {
        'textDocument': {'uri': good_uri},
        'options': {'tabSize': 2, 'insertSpaces': True}})
    check('server survives an unknown notification',
          'result' in (still_alive or {}), str(still_alive)[:160])

    check('nothing but protocol messages on stdout',
          client._stray == [], str(client._stray)[:160])

    print()
    print('=' * 72)
    print('E. Shutdown')
    print('=' * 72)

    response = client.request('shutdown')
    check('shutdown -> result', 'result' in (response or {}),
          str(response)[:120])
    client.notify('exit')
    try:
        code = client.proc.wait(timeout=5)
        check('exit -> process terminates with 0', code == 0, 'rc=%s' % code)
    except subprocess.TimeoutExpired:
        check('exit -> process terminates', False, 'still running')
        client.proc.kill()

    # A client that just closes the pipe must not leave the process hanging.
    second = Client(args.binary)
    coc_initialize(second, '/tmp')
    second.proc.stdin.close()
    try:
        code = second.proc.wait(timeout=5)
        check('closed stdin -> exits cleanly', code == 0, 'rc=%s' % code)
    except subprocess.TimeoutExpired:
        check('closed stdin -> exits cleanly', False, 'still running')
        second.proc.kill()

    print()
    print('=' * 72)
    print('TOTAL: PASS=%d  FAIL=%d' % (PASS, FAIL))
    print('=' * 72)
    if FAIL:
        return 1
    print('All LSP tests passed.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
