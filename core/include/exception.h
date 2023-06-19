#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <utility>
#include <string>

namespace hak {
    // Hakutaku
    class no_process_error : public std::exception {
    public:
        [[nodiscard]] auto what() const noexcept -> const char* override {
            return "No corresponding process found.";
        }
    };

    class recursive_maps_error : public std::exception {
    public:
        [[nodiscard]] auto what() const noexcept -> const char* override {
            return "Recursive pagemaps.";
        }
    };

    // File
    class file_error : public std::exception {
    public:
        explicit file_error(std::string message) : m_message(std::move(message)) {}

        [[nodiscard]] auto what() const noexcept -> const char* override {
            return m_message.c_str();
        }
    private:
        std::string m_message;
    };

    class file_permission_denied : public file_error {
    public:
        file_permission_denied() : file_error("No permission.") {}
    };

    class file_not_found : public file_error {
    public:
        file_not_found() : file_error("File not found.") {}
    };

    // Memory
    class memory_operate_error : public std::exception {
    public:
        explicit memory_operate_error(std::string message) : m_message(std::move(message)) {}

        explicit memory_operate_error(const char* message) : m_message(message) {}

        [[nodiscard]] auto what() const noexcept -> const char* override {
            return m_message.c_str();
        }
    private:
        std::string m_message;
    };
}

#endif // EXCEPTIONS_H