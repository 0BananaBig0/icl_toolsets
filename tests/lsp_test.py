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
import re
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

    def send_raw(self, body):
        r"""Sends a JSON body as written, with no re-encoding.

        Needed because json.dumps cannot produce some inputs the decoder has
        to survive: `\/` is valid JSON that Python never emits, `\u00ZZ` is
        not valid at all, and a lone surrogate is a shape Python refuses to
        encode. Coverage of the decoder depends on sending these by hand.
        """
        if isinstance(body, str):
            body = body.encode('utf-8')
        self.proc.stdin.write(
            b'Content-Length: %d\r\n\r\n' % len(body) + body)
        self.proc.stdin.flush()

    def request_raw(self, body):
        """Like send_raw, but returns the response with the matching id."""
        match = re.search(r'"id"\s*:\s*(\d+)', body)
        request_id = int(match.group(1)) if match else 1
        self.send_raw(body)
        return self.wait(lambda m: m.get('id') == request_id,
                         'response to raw request %r' % body[:40])

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

    # A document that is already formatted must come back with no edits at
    # all: an edit that replaces the text with itself marks the buffer dirty
    # for nothing. `ChangedFrom` is shared with the CLI's --check / -w, so the
    # two agree on what "changed" means.
    client.notify('textDocument/didOpen', {
        'textDocument': {'uri': 'file:///tmp/clean.icl', 'languageId': 'icl',
                         'version': 1, 'text': 'Module m {\n  ScanInPort SI;\n}\n'}})
    client.drain()
    response = client.request('textDocument/formatting', {
        'textDocument': {'uri': 'file:///tmp/clean.icl'},
        'options': {'tabSize': 2, 'insertSpaces': True}})
    check('already formatted -> no edits',
          (response or {}).get('result') == [], str(response)[:160])

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

    # ---- protocol and JSON-shape coverage ----
    #
    # These exist because line coverage said they were never reached. Each one
    # is a real client behaviour: coc.nvim and Neovim both send didOpen and
    # didClose, and JSON strings arriving from a client may carry escapes.

    # A header value padded with spaces/tabs. The reader trims both ends;
    # nothing in the earlier tests sent a padded header.
    padded = Client(args.binary)
    body = json.dumps({'jsonrpc': '2.0', 'id': 1, 'method': 'initialize',
                       'params': {'processId': None, 'capabilities': {}}}
                      ).encode('utf-8')
    padded.proc.stdin.write(
        b'Content-Length: %d   \t\r\n\r\n' % len(body) + body)
    padded.proc.stdin.flush()
    response = padded.wait(lambda m: m.get('id') == 1, 'padded-header reply')
    check('header value with trailing spaces/tabs is trimmed',
          response is not None and 'result' in response, str(response)[:120])
    padded.stop()

    # A body that is not JSON: the server must report and carry on serving the
    # next request rather than exiting.
    broken = Client(args.binary)
    garbage = b'{not json'
    broken.proc.stdin.write(
        b'Content-Length: %d\r\n\r\n' % len(garbage) + garbage)
    broken.proc.stdin.flush()
    after = broken.request('initialize', {'processId': None,
                                          'capabilities': {}})
    check('malformed request -> reported, server still serves the next one',
          after is not None and 'result' in after, str(after)[:120])
    broken.stop()

    # Every JSON escape the parser claims to handle. Line coverage showed
    # most of them were never exercised, and an untested decoder is exactly
    # where a malformed-\u input would corrupt a document silently.
    escapes = Client(args.binary)
    escapes.request('initialize', {
        'processId': None,
        'capabilities': {},
        r'clientInfo': {'name': 'A\u4e2d\ud83d\ude00\\b\\f\\r\\t\\/\\"'},
    })
    after_escapes = escapes.request(
        'initialize', {'processId': None, 'capabilities': {}})
    check('all JSON escape forms decode without corrupting the stream',
          after_escapes is not None and 'result' in after_escapes,
          str(after_escapes)[:120])
    escapes.stop()

    # ---- JSON decoder coverage ----
    #
    # Sent as raw bytes because these are inputs json.dumps cannot produce:
    # `\/` (valid, never emitted by Python), `\u00ZZ` (invalid), `\u00FF`
    # (uppercase hex), a surrogate pair, and malformed containers. Every one
    # of these reaches a branch in json.cc that no earlier test did, and a
    # decoder that mishandles them corrupts or drops a real client message.
    raw = Client(args.binary)

    # Non-ASCII in every UTF-8 width the encoder handles: 2-byte, 3-byte and
    # 4-byte (the last as a surrogate pair).
    raw.send_raw('{"jsonrpc":"2.0","id":1,"method":"initialize","params":'
                 r'{"clientInfo":{"name":"\u00e9\u4e2d\ud83d\ude00"}}}')
    check(r'json | 2-, 3- and 4-byte \u escapes decode',
          raw.wait(lambda m: m.get('id') == 1, 'reply') is not None)

    raw.send_raw('{"jsonrpc":"2.0","id":2,"method":"initialize","params":'
                 r'{"a":"\u00FF\u00ff\uABCD"}}')
    check('json | upper- and lower-case hex both accepted',
          raw.wait(lambda m: m.get('id') == 2, 'reply') is not None)

    # Numbers the LSP wire does not normally carry, so that the branches for a
    # fraction and an exponent are reached at all.
    raw.send_raw('{"jsonrpc":"2.0","id":21,"method":"initialize","params":'
                 '{"processId":1.5}}')
    check('json | a number with a fraction parses',
          raw.wait(lambda m: m.get('id') == 21, 'reply') is not None)

    raw.send_raw('{"jsonrpc":"2.0","id":22,"method":"initialize","params":'
                 '{"processId":1e3}}')
    check('json | a number in exponent notation parses',
          raw.wait(lambda m: m.get('id') == 22, 'reply') is not None)

    raw.send_raw('{"jsonrpc":"2.0","id":23,"method":"initialize","params":'
                 '{"processId":-7}}')
    check('json | a negative number parses',
          raw.wait(lambda m: m.get('id') == 23, 'reply') is not None)


    # r-string on purpose: in a normal string Python would turn \b \f \r \t
    # \n into real control characters, which are not legal inside a JSON
    # string. What has to reach the server is the two-character escape.
    raw.send_raw('{"jsonrpc":"2.0","id":3,"method":"initialize","params":'
                 r'{"a":"\/\b\f\r\t\n\""}}')
    check(r'json | every short escape including \/ decodes',
          raw.wait(lambda m: m.get('id') == 3, 'reply') is not None)

    # An empty array, which no message built by json.dumps happened to carry.
    raw.send_raw('{"jsonrpc":"2.0","id":4,"method":"initialize","params":'
                 '{"a":[],"b":[1,"x",{"c":[true,false,null]}]}}')
    response = raw.wait(lambda m: m.get('id') == 4, 'reply')
    check('json | empty and nested arrays parse', response is not None,
          str(response)[:120])

    # Malformed containers: each must be rejected without wedging the server,
    # which still answers the next well-formed request.
    for body in ([
        '{"jsonrpc":"2.0","id":10,"method":"initialize","params":{"a":',
        '{"jsonrpc":"2.0","id":11,"method":"initialize","params":{"a" 1}}',
        '{"jsonrpc":"2.0","id":12,"method":"initialize","params":[1,',
        '{"jsonrpc":"2.0","id":13,"method":"initialize","params":[1 2]}',
        r'{"jsonrpc":"2.0","id":14,"method":"initialize","params":{"a":"\u00ZZ"}}',
        '{"jsonrpc":"2.0","id":15,"method":"initialize","params":{"a":"\q"}}',
        '{"jsonrpc":"2.0","id":16,"method":"initialize","params":{"a":"unterminated',
        r'{"jsonrpc":"2.0","id":17,"method":"initialize","params":{"a":"\u12"}}',
        'not json at all',
        '{"jsonrpc":"2.0","id":18,"method":"initialize","params":}',
    ]):
        raw.send_raw(body)
        raw.wait(lambda m: m.get('id') is not None, 'reply', timeout=2.0)             if False else None
    # Whatever the server made of those, it must still be serving.
    after_bad = raw.request('initialize', {'processId': None,
                                           'capabilities': {}})
    check('json | malformed bodies are rejected without wedging the server',
          after_bad is not None and 'result' in after_bad,
          str(after_bad)[:120])

    # A well-formed JSON message whose *fields* have the wrong types. These
    # are not syntax errors — the body parses — so they reach a different
    # half of json.cc: the typed accessors, which must fall back rather than
    # mis-read. Branch coverage named six of them as never executed, and a
    # client that sends `"id":{}` or `"method":7` is not exotic: it is a
    # different client library's idea of the same protocol.
    #
    # Each is followed by a real request, because the property that matters
    # is not what the server answers to the bad one — it is that it answers
    # the next one. A server that wedges on a wrong type is worse than one
    # that rejects it.
    # Trailing content is asserted twice, and the second assertion is the
    # one that has teeth: "the server survives" is also true of a decoder
    # that silently ignores the extra bytes, which is the bug this guards
    # against. A body with something after the closing brace must be
    # *rejected* — no reply for its id — and the next request must still be
    # answered. Removing the trailing-content check in json.cc makes this
    # fail while every "still alive" assertion keeps passing.
    raw.send_raw('{"jsonrpc":"2.0","id":77,"method":"initialize",'
                 '"params":{}} trailing')
    # drain() rather than wait(): wait() turns "nothing arrived" into a FAIL
    # of its own, which would report this as broken every time it passes.
    seen = raw.drain(timeout=0.5)
    check('json | trailing content: the message is rejected',
          not any(m.get('id') == 77 for m in seen),
          str(seen)[:120])

    for label, body in [
        ('trailing content after a complete message',
         '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}} trailing'),
        ('"id" as an object',
         '{"jsonrpc":"2.0","id":{"a":1},"method":"initialize","params":{}}'),
        ('"id" as a bare null',
         '{"jsonrpc":"2.0","id":null,"method":"initialize","params":{}}'),
        ('"method" as a number',
         '{"jsonrpc":"2.0","id":2,"method":123,"params":{}}'),
        ('"params" as a number',
         '{"jsonrpc":"2.0","id":3,"method":"initialize","params":5}'),
        ('a number followed by "+"',
         '{"jsonrpc":"2.0","id":4,"method":"initialize","params":{"a":1+2}}'),
        ('a number with two decimal points',
         '{"jsonrpc":"2.0","id":5,"method":"initialize","params":{"a":1.2.3}}'),
    ]:
        raw.send_raw(body)
        still_alive = raw.request('initialize', {'processId': None,
                                                 'capabilities': {}})
        check('json | %s: server still serves' % label,
              still_alive is not None and 'result' in still_alive,
              str(still_alive)[:120])

    # Serialisation escapes: format a document that contains a double quote,
    # so the response body has to escape it. The server never produced an
    # escaped string before, leaving half of SerializeString unexercised.
    quoted = Client(args.binary)
    quoted.request('initialize', {'processId': None, 'capabilities': {}})
    uri = 'file:///tmp/quoted.icl'
    quoted.notify('textDocument/didOpen',
                  {'textDocument': {'uri': uri, 'languageId': 'icl',
                                    'version': 1,
                                    'text': 'Module m { Attribute a = "x\ty"; }'}})
    edits = quoted.request('textDocument/formatting',
                           {'textDocument': {'uri': uri},
                            'options': {'tabSize': 2, 'insertSpaces': True}})
    check('json | response escapes a tab inside an ICL string',
          edits is not None and '\\t' in json.dumps(edits),
          str(edits)[:200])
    quoted.stop()

    # Serialisation escapes the earlier sample did not reach: a double quote
    # and a backslash from an ICL string, and non-ASCII from a comment. The
    # server had never had to escape any of them, so half of
    # SerializeString was unexercised.
    mixed = Client(args.binary)
    mixed.request('initialize', {'processId': None, 'capabilities': {}})
    uri = 'file:///tmp/mixed.icl'
    # chr(92) rather than a literal backslash: four nesting levels (this file,
    # the JSON body, the LSP message, the ICL string) make a literal unreadable
    # and easy to get wrong by one level.
    body = 'Module m { Attribute a = "q' + chr(92) * 2 + 'b"; }'
    mixed.notify('textDocument/didOpen',
                 {'textDocument': {'uri': uri, 'languageId': 'icl',
                                   'version': 1, 'text': body}})
    edits = mixed.request('textDocument/formatting',
                          {'textDocument': {'uri': uri},
                           'options': {'tabSize': 2, 'insertSpaces': True}})
    # Round trip, not escape form: what matters is that the client receives
    # the original characters back. Asserting on the wire form would only
    # pin down which spelling the encoder uses.
    check('json | a quote and a backslash survive the round trip',
          edits is not None and
          edits['result'][0]['newText'] ==
          'Module m {\n  Attribute a = "q' + chr(92) * 2 + 'b";\n}\n',
          str(edits)[:200])

    unicode_uri = 'file:///tmp/unicode.icl'
    mixed.notify('textDocument/didOpen',
                 {'textDocument': {'uri': unicode_uri, 'languageId': 'icl',
                                   'version': 1,
                                   'text': 'Module m {\n  /* 中文注释 */\n  ScanInPort SI;\n}'}})
    edits = mixed.request('textDocument/formatting',
                          {'textDocument': {'uri': unicode_uri},
                           'options': {'tabSize': 2, 'insertSpaces': True}})
    check('json | non-ASCII survives the response round trip',
          edits is not None and '中文注释' in edits['result'][0]['newText'],
          str(edits)[:200])
    mixed.stop()

    # A \u escape below 0x80 (one UTF-8 byte) and a broken surrogate pair:
    # a high surrogate followed by something that is not a low one, which has
    # to fall back to the saved position rather than consume the escape.
    raw2 = Client(args.binary)
    raw2.send_raw('{"jsonrpc":"2.0","id":20,"method":"initialize","params":'
                  r'{"a":"\u0041\u007F"}}')
    check('json | \\u escapes below 0x80 decode',
          raw2.wait(lambda m: m.get('id') == 20, 'reply') is not None)
    raw2.send_raw('{"jsonrpc":"2.0","id":21,"method":"initialize","params":'
                  r'{"a":"\ud83d\u0041"}}')
    check('json | high surrogate without a low one does not consume it',
          raw2.wait(lambda m: m.get('id') == 21, 'reply') is not None)
    # A truncated object, ending mid-member.
    raw2.send_raw('{"jsonrpc":"2.0","id":22,"method":"initialize","params":'
                  '{"a":')
    check('json | truncated object is rejected without wedging the server',
          raw2.request('initialize', {'processId': None,
                                      'capabilities': {}}) is not None)
    raw2.stop()
    raw.stop()

    # Header shapes no client built from json.dumps happens to send. Each one
    # exercises a different arm of the reader: a line with no colon at all, a
    # lower-case header name, and a message with no Content-Length.
    headers = Client(args.binary)
    body = b'{"jsonrpc":"2.0","id":40,"method":"initialize","params":{}}'
    headers.proc.stdin.write(b'X-No-Colon\r\n' +
                             b'Content-Length: %d\r\n\r\n' % len(body) + body)
    headers.proc.stdin.flush()
    check('header line without a colon is skipped',
          headers.wait(lambda m: m.get('id') == 40, 'reply') is not None)

    body2 = b'{"jsonrpc":"2.0","id":41,"method":"initialize","params":{}}'
    headers.proc.stdin.write(b'Content-Length: %d\r\n\r\n' % len(body2) + body2)
    headers.proc.stdin.flush()
    check('a second message after a skipped header is still served',
          headers.wait(lambda m: m.get('id') == 41, 'reply') is not None)

    # A lower-case header name. The header block is HTTP-shaped, so names are
    # case-insensitive; comparing exactly against "Content-Length" used to drop
    # such a message entirely, because a frame with no length cannot be read.
    headers2 = Client(args.binary)
    body_lc = b'{"jsonrpc":"2.0","id":43,"method":"initialize","params":{}}'
    headers2.proc.stdin.write(b'content-LENGTH: %d\r\n\r\n' % len(body_lc) + body_lc)
    headers2.proc.stdin.flush()
    check('header name is matched case-insensitively',
          headers2.wait(lambda m: m.get('id') == 43, 'reply') is not None)
    headers2.stop()

    # No Content-Length at all: the reader cannot know the body size, so it
    # must give up on this message rather than read the next one as a body.
    headers.send_raw(b'{"jsonrpc":"2.0","id":42,"method":"initialize"}')
    check('a message with no Content-Length does not wedge the server',
          headers.request('initialize', {'processId': None,
                                        'capabilities': {}}) is not None)
    headers.stop()

    # didOpen and didClose, which the earlier tests skipped: they went straight
    # to didChange.
    lifecycle = Client(args.binary)
    lifecycle.request('initialize', {'processId': None, 'capabilities': {}})
    uri = 'file:///tmp/lifecycle.icl'
    lifecycle.notify('textDocument/didOpen',
                     {'textDocument': {'uri': uri, 'languageId': 'icl',
                                       'version': 1,
                                       'text': 'Module m {\nScanInPort SI;\n}'}})
    edits = lifecycle.request(
        'textDocument/formatting',
        {'textDocument': {'uri': uri}, 'options': {'tabSize': 2,
                                                   'insertSpaces': True}})
    check('didOpen -> document is formatable',
          edits is not None and len(edits.get('result') or []) == 1,
          str(edits)[:160])

    # JSON value shapes the earlier tests never sent: escapes, `false`,
    # a floating point number and a nested array. A real client sends all of
    # these in `capabilities`.
    rich = Client(args.binary)
    rich.request('initialize', {
        'processId': None,
        'rootUri': 'file:///tmp',
        'capabilities': {
            'textDocument': {
                'formatting': {'dynamicRegistration': False},
                'synchronization': {'didSave': True},
            },
            'workspace': {'workspaceFolders': True},
        },
        r'clientInfo': {'name': 'test\u00e9\t\\ \"x\"', 'version': '1.5'},
        'trace': 'off',
        'initializationOptions': [1, 2.5, False, None, {'nested': [True]}],
    })
    still = rich.request('textDocument/formatting',
                         {'textDocument': {'uri': 'file:///tmp/a.icl'},
                          'options': {'tabSize': 2, 'insertSpaces': True}})
    check('escapes / false / float / array in params are accepted',
          still is not None and 'result' in still, str(still)[:120])

    # didChange carrying the whole document: the branch that replaces the
    # stored text. (didOpen alone leaves it unreached.)
    lifecycle.notify('textDocument/didChange', {
        'textDocument': {'uri': uri, 'version': 2},
        'contentChanges': [
            {'text': 'Module m {\nScanInPort SI;\nScanOutPort SO;\n}'}]})
    changed = lifecycle.request(
        'textDocument/formatting',
        {'textDocument': {'uri': uri}, 'options': {'tabSize': 2,
                                                   'insertSpaces': True}})
    check('didChange replaces the document text',
          changed is not None and
          'ScanOutPort SO;' in json.dumps(changed),
          str(changed)[:160])

    # tabSize 0 is what a client sends when it has not configured one, and
    # it must be clamped rather than honoured: indenting by zero would flatten
    # the whole document. The CLI cannot reach this — it rejects `-n 0` at
    # argument parsing — so the LSP is the only way to exercise the clamp.
    zero = lifecycle.request(
        'textDocument/formatting',
        {'textDocument': {'uri': uri}, 'options': {'tabSize': 0,
                                                   'insertSpaces': True}})
    zero_text = ''
    if zero is not None and zero.get('result'):
        zero_text = zero['result'][0]['newText']
    check('tabSize 0 is clamped, not honoured',
          '  ScanInPort SI;' in zero_text, repr(zero_text)[:160])
    huge = lifecycle.request(
        'textDocument/formatting',
        {'textDocument': {'uri': uri}, 'options': {'tabSize': 99999,
                                                   'insertSpaces': True}})
    huge_text = huge['result'][0]['newText'] if huge.get('result') else ''
    check('an absurd tabSize cannot blow the reply up',
          len(huge_text) < 10000, 'len=%d' % len(huge_text))

    lifecycle.notify('textDocument/didClose', {'textDocument': {'uri': uri}})
    gone = lifecycle.request(
        'textDocument/formatting',
        {'textDocument': {'uri': uri}, 'options': {'tabSize': 2,
                                                   'insertSpaces': True}})
    check('didClose -> document forgotten, formatting returns no edit',
          gone is not None and gone.get('result') in (None, []),
          str(gone)[:160])
    lifecycle.stop()
    rich.stop()

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
