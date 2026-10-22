// Copyright 2024 The ICL Format Authors.
//
// SPDX-License-Identifier: MIT
// See LICENSE in the project root for license information.
//
// icl-format — format IEEE 1687-2014 (IJTAG) ICL files.
//
// Pipeline: text --(flex/bison)--> AST --(layout)--> lines --(render)--> text
//
// Exit codes:
//   0  all inputs formatted (or already formatted, with --check)
//   1  --check found at least one file that would change
//   2  a syntax error, an unreadable file, or a bad command line

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "format_job.h"

namespace {

const char kUsage[] =
    "usage: icl-format [options] FILE...\n"
    "\n"
    "Formats IEEE 1687-2014 (IJTAG) Instrument Connectivity Language files.\n"
    "Reads from stdin when no FILE is given.\n"
    "\n"
    "options:\n"
    "  -i, --stdin          read from stdin (explicit)\n"
    "  -w, --write          rewrite files in place (default: print to stdout)\n"
    "  -o FILE              write the result to FILE (single input only)\n"
    "  -n, --indent N       spaces per indentation level (default: 2)\n"
    "      --check          do not write; exit 1 if any file would change\n"
    "      --ast            print the abstract syntax tree and exit\n"
    "      --version        print version and exit\n"
    "  -h, --help           print this message and exit\n"
    "\n"
    "exit codes:\n"
    "  0  success            1  --check found unformatted files\n"
    "  2  syntax error, unreadable file, or bad usage\n";

struct Options {
  bool write = false;
  bool check = false;
  bool from_stdin = false;
  bool dump_ast = false;
  std::string output_path;
  int indent_width = 2;
  std::vector<std::string> inputs;
};

bool ParseArgs(int argc, char** argv, Options* options, std::string* error) {
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      std::cout << kUsage;
      exit(0);
    } else if (arg == "--version") {
      std::cout << "icl-format " << ICL_FORMAT_VERSION << "\n";
      exit(0);
    } else if (arg == "-w" || arg == "--write") {
      options->write = true;
    } else if (arg == "--check") {
      options->check = true;
    } else if (arg == "-i" || arg == "--stdin") {
      options->from_stdin = true;
    } else if (arg == "--ast") {
      options->dump_ast = true;
    } else if (arg == "-n" || arg == "--indent") {
      if (i + 1 >= argc) {
        *error = "missing value for " + arg;
        return false;
      }
      options->indent_width = std::atoi(argv[++i]);
      if (options->indent_width <= 0) {
        *error = "--indent must be a positive integer";
        return false;
      }
    } else if (arg == "-o") {
      if (i + 1 >= argc) {
        *error = "missing value for -o";
        return false;
      }
      options->output_path = argv[++i];
    } else if (!arg.empty() && arg[0] == '-') {
      *error = "unknown option: " + arg;
      return false;
    } else {
      options->inputs.push_back(arg);
    }
  }

  if (options->write && options->check) {
    *error = "--write and --check are mutually exclusive";
    return false;
  }
  if (!options->output_path.empty() &&
      (options->inputs.size() > 1 || options->write)) {
    *error = "-o requires exactly one input and cannot be combined with -w";
    return false;
  }
  if (options->inputs.empty() && !options->from_stdin) {
    // No arguments at all: read stdin. This makes `icl-format < file.icl`
    // work, which is the usual way to plug it into an editor.
    options->from_stdin = true;
  }
  return true;
}


// One unit of work: a named file, or stdin. |from_file| doubles as "the result
// may be written back", which stdin never allows.
struct Job {
  std::string name;
  std::string text;  // Only filled for stdin.
  bool from_file;
};

bool WriteFile(const std::string& path, const std::string& data) {
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out) {
    std::cerr << "icl-format: cannot write '" << path << "'\n";
    return false;
  }
  out << data;
  return true;
}

// Reads a whole file. This stays in the CLI because it is the one step the
// LSP never performs: the server holds documents in memory already.
//
// Returns false when the file cannot be opened or is not text — binary input
// must never be silently reformatted.
bool ReadInput(const std::string& path, std::string* text,
               std::string* error) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    *error = "cannot open '" + path + "'";
    return false;
  }
  std::ostringstream buffer;
  buffer << stream.rdbuf();
  *text = buffer.str();
  if (text->find('\0') != std::string::npos) {
    *error = "not a text file (contains NUL bytes): '" + path + "'";
    return false;
  }
  return true;
}

// Dispatches one formatted result according to the mode. Returns false only
// when a write failed.
bool Emit(const std::string& name, const std::string& original,
          const icl::FormatOutcome& outcome, bool writable,
          const Options& options, bool* would_change) {
  if (options.check) {
    if (outcome.ChangedFrom(original)) {
      std::cout << name << "\n";
      *would_change = true;
    }
    return true;
  }
  if (!options.output_path.empty()) {
    return WriteFile(options.output_path, outcome.text);
  }
  if (options.write && writable) {
    // Only touch the file when the content actually changes; this keeps
    // mtimes (and build systems) quiet on already-formatted trees.
    return outcome.ChangedFrom(original) ? WriteFile(name, outcome.text)
                                        : true;
  }
  std::cout << outcome.text;
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  Options options;
  std::string error;
  if (!ParseArgs(argc, argv, &options, &error)) {
    std::cerr << "icl-format: " << error << "\n";
    return 2;
  }

  // Everything past this point that is not about *files* is shared with the
  // LSP server: building the request, running the pipeline, and phrasing the
  // failure. See format_job.h.
  icl::FormatRequest request;
  request.indent_width = options.indent_width;
  request.with_ast_dump = options.dump_ast;

  // stdin and named files go through the same pipeline and the same output
  // handling; the only difference is where the text comes from and whether
  // the result may be written back.
  std::vector<Job> jobs;
  if (options.from_stdin) {
    std::string text((std::istreambuf_iterator<char>(std::cin)),
                     std::istreambuf_iterator<char>());
    jobs.push_back({"<stdin>", std::move(text), false});
  } else {
    for (const std::string& path : options.inputs) {
      jobs.push_back({path, std::string(), true});
    }
  }

  bool would_change = false;
  bool had_error = false;
  for (const Job& job : jobs) {
    // Reading is the one genuinely file-shaped step, so it stays here. A
    // stdin job already carries its text.
    std::string original = job.text;
    if (job.from_file && !ReadInput(job.name, &original, &error)) {
      std::cerr << "icl-format: " << error << "\n";
      had_error = true;
      continue;
    }

    const icl::FormatOutcome outcome = icl::FormatDocument(original, request);
    if (!outcome.ok) {
      // A stdin job is named "<stdin>", so the diagnostic needs no case here.
      std::cerr << "icl-format: " << outcome.Diagnostic(job.name) << "\n";
      had_error = true;
      continue;
    }
    if (options.dump_ast) {
      std::cout << outcome.ast_dump;
      continue;
    }
    if (!Emit(job.name, original, outcome, job.from_file, options,
              &would_change)) {
      had_error = true;
    }
  }

  if (had_error) return 2;
  if (options.check && would_change) return 1;
  return 0;
}
