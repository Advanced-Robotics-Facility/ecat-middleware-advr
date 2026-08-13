#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

class Console
{
public:

    explicit Console(std::ostream& os = std::cout,
                     std::size_t width = 80)
        : os_(os),
          width_(width)
    {
    }

    void clear()
    {
        os_ << "\033[2J\033[H";
    }

    void title(std::string_view text)
    {
        separator('=');
        center(text);
        separator('=');
        blank();
    }

    void section(std::string_view text)
    {
        separator('-');
        os_ << text << '\n';
        separator('-');
    }

    template<typename T>
    void field(std::string_view key,
            const T& value)
    {
        os_
            << std::left
            << std::setw(label_width_)
            << key
            << " : "
            << value
            << '\n';
    }

    void separator(char c = '-')
    {
        os_ << std::string(width_, c) << '\n';
    }

    void blank()
    {
        os_ << '\n';
    }

    void print_indented(std::string_view text, std::size_t spaces = 4)
    {
        std::string prefix(spaces, ' ');
        std::istringstream in(text.data());
        std::string line;

        while (std::getline(in, line))
            os_ << prefix << line << '\n';
    }

    static std::string progress_bar(std::size_t value,
                                    std::size_t capacity,
                                    std::size_t width)
    {
        const auto filled = capacity
            ? value * width / capacity
            : 0;

        return std::string(filled, '#') +
            std::string(width - filled, '.');
    }

private:

    void center(std::string_view text)
    {
        if (text.size() >= width_)
        {
            os_ << text << '\n';
            return;
        }

        const std::size_t left = (width_ - text.size()) / 2;

        os_
            << std::string(left, ' ')
            << text
            << '\n';
    }

    std::ostream& os_;

    std::size_t width_{80};
    std::size_t label_width_{18};
};
