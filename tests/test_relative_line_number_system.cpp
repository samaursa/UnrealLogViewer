#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/relative_line_number_system.h"
#include <memory>

using namespace ue_log;

// Test relative line number calculation
TEST_CASE("RelativeLineNumberSystem - CalculateRelativeNumbers BasicCase", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Test with current selection at index 5, showing 5 entries starting from index 3
    int current_selection = 5;
    int visible_start = 3;
    int visible_count = 5;
    
    auto result = system->CalculateRelativeNumbers(current_selection, visible_start, visible_count);
    
    REQUIRE(result.size() == 5);
    
    // Check each entry
    CHECK(result[0].absolute_number == 3);
    CHECK(result[0].relative_number == -2);  // 3 - 5 = -2
    CHECK(result[0].is_current_line == false);
    
    CHECK(result[1].absolute_number == 4);
    CHECK(result[1].relative_number == -1);  // 4 - 5 = -1
    CHECK(result[1].is_current_line == false);
    
    CHECK(result[2].absolute_number == 5);
    CHECK(result[2].relative_number == 0);   // 5 - 5 = 0
    CHECK(result[2].is_current_line == true);
    
    CHECK(result[3].absolute_number == 6);
    CHECK(result[3].relative_number == 1);   // 6 - 5 = 1
    CHECK(result[3].is_current_line == false);
    
    CHECK(result[4].absolute_number == 7);
    CHECK(result[4].relative_number == 2);   // 7 - 5 = 2
    CHECK(result[4].is_current_line == false);
}

TEST_CASE("RelativeLineNumberSystem - CalculateRelativeNumbers CurrentAtTop", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Test with current selection at the top of visible area
    int current_selection = 0;
    int visible_start = 0;
    int visible_count = 3;
    
    auto result = system->CalculateRelativeNumbers(current_selection, visible_start, visible_count);
    
    REQUIRE(result.size() == 3);
    
    CHECK(result[0].absolute_number == 0);
    CHECK(result[0].relative_number == 0);
    CHECK(result[0].is_current_line == true);
    
    CHECK(result[1].absolute_number == 1);
    CHECK(result[1].relative_number == 1);
    CHECK(result[1].is_current_line == false);
    
    CHECK(result[2].absolute_number == 2);
    CHECK(result[2].relative_number == 2);
    CHECK(result[2].is_current_line == false);
}

TEST_CASE("RelativeLineNumberSystem - CalculateRelativeNumbers CurrentAtBottom", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Test with current selection at the bottom of visible area
    int current_selection = 10;
    int visible_start = 8;
    int visible_count = 3;
    
    auto result = system->CalculateRelativeNumbers(current_selection, visible_start, visible_count);
    
    REQUIRE(result.size() == 3);
    
    CHECK(result[0].absolute_number == 8);
    CHECK(result[0].relative_number == -2);  // 8 - 10 = -2
    CHECK(result[0].is_current_line == false);
    
    CHECK(result[1].absolute_number == 9);
    CHECK(result[1].relative_number == -1);  // 9 - 10 = -1
    CHECK(result[1].is_current_line == false);
    
    CHECK(result[2].absolute_number == 10);
    CHECK(result[2].relative_number == 0);   // 10 - 10 = 0
    CHECK(result[2].is_current_line == true);
}

TEST_CASE("RelativeLineNumberSystem - CalculateRelativeNumbers EmptyRange", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Test with zero visible count
    int current_selection = 5;
    int visible_start = 3;
    int visible_count = 0;
    
    auto result = system->CalculateRelativeNumbers(current_selection, visible_start, visible_count);
    
    CHECK(result.size() == 0);
}

// Test vim-style navigation command parsing
TEST_CASE("RelativeLineNumberSystem - HandleNavigationInput SimpleCommands", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    int jump_distance;
    char direction;
    
    // Test simple 'j' command
    CHECK(system->HandleNavigationInput("j", jump_distance, direction) == true);
    CHECK(jump_distance == 1);
    CHECK(direction == 'j');
    
    // Test simple 'k' command
    CHECK(system->HandleNavigationInput("k", jump_distance, direction) == true);
    CHECK(jump_distance == 1);
    CHECK(direction == 'k');
}

TEST_CASE("RelativeLineNumberSystem - HandleNavigationInput NumberedCommands", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    int jump_distance;
    char direction;
    
    // Test "5j" command
    CHECK(system->HandleNavigationInput("5j", jump_distance, direction) == true);
    CHECK(jump_distance == 5);
    CHECK(direction == 'j');
    
    // Test "10k" command
    CHECK(system->HandleNavigationInput("10k", jump_distance, direction) == true);
    CHECK(jump_distance == 10);
    CHECK(direction == 'k');
    
    // Test "123j" command
    CHECK(system->HandleNavigationInput("123j", jump_distance, direction) == true);
    CHECK(jump_distance == 123);
    CHECK(direction == 'j');
}

TEST_CASE("RelativeLineNumberSystem - HandleNavigationInput InvalidCommands", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    int jump_distance;
    char direction;
    
    // Test empty string
    CHECK(system->HandleNavigationInput("", jump_distance, direction) == false);
    
    // Test invalid command character
    CHECK(system->HandleNavigationInput("5x", jump_distance, direction) == false);
    CHECK(system->HandleNavigationInput("l", jump_distance, direction) == false);
    
    // Test invalid numeric part
    CHECK(system->HandleNavigationInput("abcj", jump_distance, direction) == false);
    CHECK(system->HandleNavigationInput("5a3j", jump_distance, direction) == false);
    
    // Test zero count
    CHECK(system->HandleNavigationInput("0j", jump_distance, direction) == false);
    
    // Test negative count
    CHECK(system->HandleNavigationInput("-5j", jump_distance, direction) == false);
}

TEST_CASE("RelativeLineNumberSystem - HandleNavigationInput EdgeCases", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    int jump_distance;
    char direction;
    
    // Test single digit commands
    CHECK(system->HandleNavigationInput("1j", jump_distance, direction) == true);
    CHECK(jump_distance == 1);
    CHECK(direction == 'j');
    
    CHECK(system->HandleNavigationInput("9k", jump_distance, direction) == true);
    CHECK(jump_distance == 9);
    CHECK(direction == 'k');
    
    // Test large numbers
    CHECK(system->HandleNavigationInput("999j", jump_distance, direction) == true);
    CHECK(jump_distance == 999);
    CHECK(direction == 'j');
}

// Test relative number formatting
TEST_CASE("RelativeLineNumberSystem - FormatRelativeNumber CurrentLine", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Test current line formatting
    std::string result = system->FormatRelativeNumber(0, true, 4);
    CHECK(result == "   0");
}

TEST_CASE("RelativeLineNumberSystem - FormatRelativeNumber PositiveNumbers", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Test positive relative numbers (lines below current) - no + sign needed for vim users
    std::string result1 = system->FormatRelativeNumber(1, false, 4);
    CHECK(result1 == "   1");
    
    std::string result5 = system->FormatRelativeNumber(5, false, 4);
    CHECK(result5 == "   5");
    
    std::string result10 = system->FormatRelativeNumber(10, false, 5);
    CHECK(result10 == "   10");
}

TEST_CASE("RelativeLineNumberSystem - FormatRelativeNumber NegativeNumbers", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Test negative relative numbers (lines above current) - show absolute value, no - sign needed for vim users
    std::string result1 = system->FormatRelativeNumber(-1, false, 4);
    CHECK(result1 == "   1");
    
    std::string result5 = system->FormatRelativeNumber(-5, false, 4);
    CHECK(result5 == "   5");
    
    std::string result10 = system->FormatRelativeNumber(-10, false, 5);
    CHECK(result10 == "   10");
}

TEST_CASE("RelativeLineNumberSystem - FormatRelativeNumber DifferentWidths", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Test different width formatting
    std::string result3 = system->FormatRelativeNumber(1, false, 3);
    CHECK(result3 == "  1");
    
    std::string result6 = system->FormatRelativeNumber(1, false, 6);
    CHECK(result6 == "     1");
    
    std::string result1 = system->FormatRelativeNumber(0, true, 1);
    CHECK(result1 == "0");
}

// Integration tests
TEST_CASE("RelativeLineNumberSystem - Integration TypicalUsage", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Simulate a typical usage scenario
    int current_selection = 10;
    int visible_start = 8;
    int visible_count = 5;
    
    // Calculate relative numbers
    auto line_info = system->CalculateRelativeNumbers(current_selection, visible_start, visible_count);
    
    // Format each line number
    std::vector<std::string> formatted_numbers;
    for (const auto& info : line_info) {
        formatted_numbers.push_back(
            system->FormatRelativeNumber(info.relative_number, info.is_current_line, 4)
        );
    }
    
    // Verify the formatted output
    REQUIRE(formatted_numbers.size() == 5);
    CHECK(formatted_numbers[0] == "   2");  // Line 8, relative -2 (shows absolute value)
    CHECK(formatted_numbers[1] == "   1");  // Line 9, relative -1 (shows absolute value)
    CHECK(formatted_numbers[2] == "   0");  // Line 10, current line
    CHECK(formatted_numbers[3] == "   1");  // Line 11, relative +1 (shows absolute value)
    CHECK(formatted_numbers[4] == "   2");  // Line 12, relative +2 (shows absolute value)
}

TEST_CASE("RelativeLineNumberSystem - Integration NavigationCommands", "[RelativeLineNumberSystem]") {
    auto system = std::make_unique<RelativeLineNumberSystem>();
    
    // Test a sequence of navigation commands
    int jump_distance;
    char direction;
    
    // Simulate user typing "15j"
    CHECK(system->HandleNavigationInput("15j", jump_distance, direction) == true);
    CHECK(jump_distance == 15);
    CHECK(direction == 'j');
    
    // Simulate user typing "3k"
    CHECK(system->HandleNavigationInput("3k", jump_distance, direction) == true);
    CHECK(jump_distance == 3);
    CHECK(direction == 'k');
    
    // Simulate user typing just "j"
    CHECK(system->HandleNavigationInput("j", jump_distance, direction) == true);
    CHECK(jump_distance == 1);
    CHECK(direction == 'j');
}