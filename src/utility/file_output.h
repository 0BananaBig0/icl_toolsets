// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_UTILITY_FILE_OUTPUT_H_
#define ICL_TOOLSETS_SRC_UTILITY_FILE_OUTPUT_H_
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <utility>
namespace toolsets {
class StagedTextFile {
 public:
  StagedTextFile() = default;
  StagedTextFile(const StagedTextFile&) = delete;
  StagedTextFile& operator=(const StagedTextFile&) = delete;
  ~StagedTextFile() {
    if (!temporary_.empty()) ::unlink(temporary_.c_str());
  }
  bool Prepare(const std::string& path, const std::string& data,
               std::string* message) {
    if (!temporary_.empty()) {
      *message = "output is already staged";
      return false;
    }
    const auto error = [&](int code) {
      *message = "cannot write '" + path + "': " + std::strerror(code);
      return false;
    };
    std::error_code status_error;
    auto destination = std::filesystem::canonical(path, status_error);
    if (status_error) {
      std::error_code link_error;
      // Never replace a dangling symlink with a different kind of file.
      if (std::filesystem::is_symlink(path, link_error)) {
        return error(status_error.value());
      }
      destination = std::filesystem::weakly_canonical(path, status_error);
      if (status_error) return error(status_error.value());
    }

    struct stat original{};
    const bool exists = ::stat(destination.c_str(), &original) == 0;
    if (!exists && errno != ENOENT) return error(errno);
    if (exists) {
      if (!S_ISREG(original.st_mode)) return error(EINVAL);
      // Renaming only requires directory permissions; respect file permissions.
      const int check = ::open(destination.c_str(),
                               O_WRONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK);
      if (check < 0) return error(errno);
      const int result = ::fstat(check, &original);
      const int code = errno;
      if (::close(check) != 0) return error(errno);
      if (result != 0) return error(code);
      if (!S_ISREG(original.st_mode)) return error(EINVAL);
    }

    std::string temporary =
        (destination.parent_path() / ".icl-toolsets-XXXXXX").string();
    int descriptor = ::mkstemp(temporary.data());
    if (descriptor < 0) return error(errno);
    const auto fail = [&] {
      const int code = errno;
      if (descriptor >= 0) ::close(descriptor);
      ::unlink(temporary.c_str());
      return error(code);
    };
    size_t offset = 0;
    while (offset < data.size()) {
      const auto written =
          ::write(descriptor, data.data() + offset, data.size() - offset);
      if (written < 0) {
        if (errno == EINTR) continue;
        return fail();
      }
      if (written == 0) {
        errno = EIO;
        return fail();
      }
      offset += static_cast<size_t>(written);
    }

    mode_t mode;
    if (exists) {
      struct stat created{};
      if (::fstat(descriptor, &created) != 0) return fail();
      if ((created.st_uid != original.st_uid ||
           created.st_gid != original.st_gid) &&
          ::fchown(descriptor, original.st_uid, original.st_gid) != 0) {
        return fail();
      }
      mode = original.st_mode & 07777;
    } else {
      const mode_t mask = ::umask(0);
      ::umask(mask);
      mode = 0666 & ~mask;
    }
    if (::fchmod(descriptor, mode) != 0 || ::fsync(descriptor) != 0) {
      return fail();
    }
    const int closed = ::close(descriptor);
    descriptor = -1;
    if (closed != 0) return fail();
    temporary_ = std::move(temporary);
    destination_ = destination.string();
    return true;
  }
  bool Publish(std::string* message) {
    if (temporary_.empty()) {
      *message = "output was not staged";
      return false;
    }
    if (::rename(temporary_.c_str(), destination_.c_str()) != 0) {
      *message =
          "cannot publish '" + destination_ + "': " + std::strerror(errno);
      return false;
    }
    temporary_.clear();
    return true;
  }

 private:
  std::string temporary_, destination_;
};
inline bool WriteTextFile(const std::string& path, const std::string& data,
                          std::string* message) {
  StagedTextFile output;
  return output.Prepare(path, data, message) && output.Publish(message);
}

}  // namespace toolsets
#endif  // ICL_TOOLSETS_SRC_UTILITY_FILE_OUTPUT_H_
