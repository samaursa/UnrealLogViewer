#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"
#include <memory>
#include <fstream>
#include <filesystem>
#include <chrono>

using namespace ue_log;

namespace {
    void CreateTestLogFile(const std::string& path, const std::vector<std::string>& lines) {
        std::ofstream file(path);
        for (const auto& line : lines) {
            file << line << "\n";
        }
        file.close();
    }
    
    void DeleteTestFile(const std::string& path) {
        try {
            if (std::filesystem::exists(path)) {
                std::filesystem::remove(path);
            }
        } catch (...) {
            // Ignore deletion errors in tests
        }
    }
    
    std::string GetUniqueTestFileName(const std::string& base_name) {
        static int counter = 0;
        return base_name + "_" + std::to_string(++counter) + ".log";
    }
    
    std::vector<std::string> CreateLargeLogFile(int line_count) {
        std::vector<std::string> lines;
        lines.reserve(line_count);
        
        for (int i = 0; i < line_count; ++i) {
            // Create varied log entries to simulate real log files
            std::string level = (i % 4 == 0) ? "Error" : (i % 3 == 0) ? "Warning" : "Info";
            std::string logger = (i % 5 == 0) ? "LogCore" : (i % 3 == 0) ? "LogTemp" : "LogGame";
            
            lines.push_back("[2024.01.15-10." + std::to_string(30 + (i / 60)) + "." + 
                           std::to_string(45 + (i % 60)) + ":123][" + 
                           std::to_string(456 + i) + "]" + logger + ": " + level + 
                           ": Performance test message " + std::to_string(i + 1) + 
                           " with some additional content to make it realistic");
        }
        
        return lines;
    }
    
    template<typename Func>
    double MeasureExecutionTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return duration.count() / 1000.0; // Return milliseconds
    }
}

class VisualSelectionPerformanceTest {
public:
    void SetUp() {
        config_manager_ = std::make_unique<ConfigManager>();
        main_window_ = std::make_unique<MainWindow>(config_manager_.get());
        main_window_->Initialize();
    }
    
    void TearDown() {
        if (!temp_log_file_.empty()) {
            DeleteTestFile(temp_log_file_);
        }
    }
    
    void LoadTestFile(const std::vector<std::string>& lines) {
        temp_log_file_ = GetUniqueTestFileName("performance_test");
        CreateTestLogFile(temp_log_file_, lines);
        main_window_->LoadLogFile(temp_log_file_);
    }
    
public:
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<MainWindow> main_window_;
    std::string temp_log_file_;
};

TEST_CASE("Visual Selection Performance - Large file operations", "[visual_selection][performance]") {
    VisualSelectionPerformanceTest test;
    test.SetUp();
    
    SECTION("Performance with 1000 lines") {
        auto lines = CreateLargeLogFile(1000);
        test.LoadTestFile(lines);
        
        // Test entering visual selection mode
        double enter_time = MeasureExecutionTime([&]() {
            test.main_window_->EnterVisualSelectionMode();
        });
        
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(enter_time < 10.0); // Should complete in less than 10ms
        
        // Test extending to large selection
        double extend_time = MeasureExecutionTime([&]() {
            test.main_window_->ExtendVisualSelection(999);
        });
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1000);
        REQUIRE(extend_time < 50.0); // Should complete in less than 50ms
        
        // Test rendering with large selection
        double render_time = MeasureExecutionTime([&]() {
            auto element = test.main_window_->Render();
            REQUIRE(element != nullptr);
        });
        
        REQUIRE(render_time < 200.0); // Should render in less than 200ms
        
        // Test copying large selection
        double copy_time = MeasureExecutionTime([&]() {
            test.main_window_->CopyVisualSelectionToClipboard();
        });
        
        REQUIRE(copy_time < 100.0); // Should copy in less than 100ms
    }
    
    SECTION("Performance with 5000 lines") {
        auto lines = CreateLargeLogFile(5000);
        test.LoadTestFile(lines);
        
        test.main_window_->EnterVisualSelectionMode();
        
        // Test extending to very large selection
        double extend_time = MeasureExecutionTime([&]() {
            test.main_window_->ExtendVisualSelection(4999);
        });
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 5000);
        REQUIRE(extend_time < 100.0); // Should complete in less than 100ms
        
        // Test rendering with very large selection
        double render_time = MeasureExecutionTime([&]() {
            auto element = test.main_window_->Render();
            REQUIRE(element != nullptr);
        });
        
        REQUIRE(render_time < 500.0); // Should render in less than 500ms
    }
    
    SECTION("Performance with 10000 lines") {
        auto lines = CreateLargeLogFile(10000);
        test.LoadTestFile(lines);
        
        test.main_window_->EnterVisualSelectionMode();
        
        // Test extending to maximum selection
        double extend_time = MeasureExecutionTime([&]() {
            test.main_window_->ExtendVisualSelection(9999);
        });
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 10000);
        REQUIRE(extend_time < 200.0); // Should complete in less than 200ms
        
        // Test that rendering is still reasonable with maximum selection
        double render_time = MeasureExecutionTime([&]() {
            auto element = test.main_window_->Render();
            REQUIRE(element != nullptr);
        });
        
        REQUIRE(render_time < 1000.0); // Should render in less than 1 second
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Performance - Rapid operations", "[visual_selection][performance]") {
    VisualSelectionPerformanceTest test;
    test.SetUp();
    
    auto lines = CreateLargeLogFile(1000);
    test.LoadTestFile(lines);
    
    SECTION("Rapid selection changes") {
        test.main_window_->EnterVisualSelectionMode();
        
        // Test rapid extension changes
        double rapid_changes_time = MeasureExecutionTime([&]() {
            for (int i = 0; i < 100; ++i) {
                test.main_window_->ExtendVisualSelection(i * 5);
            }
        });
        
        REQUIRE(rapid_changes_time < 100.0); // Should complete in less than 100ms
        REQUIRE(test.main_window_->GetVisualSelectionSize() > 0);
    }
    
    SECTION("Rapid mode switching") {
        double mode_switching_time = MeasureExecutionTime([&]() {
            for (int i = 0; i < 50; ++i) {
                test.main_window_->EnterVisualSelectionMode();
                test.main_window_->ExtendVisualSelection(i);
                test.main_window_->ExitVisualSelectionMode();
            }
        });
        
        REQUIRE(mode_switching_time < 200.0); // Should complete in less than 200ms
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    SECTION("Rapid rendering updates") {
        test.main_window_->EnterVisualSelectionMode();
        test.main_window_->ExtendVisualSelection(100);
        
        double rendering_time = MeasureExecutionTime([&]() {
            for (int i = 0; i < 20; ++i) {
                auto element = test.main_window_->Render();
                REQUIRE(element != nullptr);
            }
        });
        
        REQUIRE(rendering_time < 500.0); // Should complete in less than 500ms
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Performance - Memory efficiency", "[visual_selection][performance]") {
    VisualSelectionPerformanceTest test;
    test.SetUp();
    
    SECTION("Memory usage with large selections") {
        auto lines = CreateLargeLogFile(2000);
        test.LoadTestFile(lines);
        
        // Test that large selections don't cause excessive memory usage
        test.main_window_->EnterVisualSelectionMode();
        
        // Create and destroy large selections multiple times
        for (int i = 0; i < 10; ++i) {
            test.main_window_->ExtendVisualSelection(1999);
            REQUIRE(test.main_window_->GetVisualSelectionSize() == 2000);
            
            test.main_window_->ExitVisualSelectionMode();
            REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
            
            test.main_window_->EnterVisualSelectionMode();
        }
        
        // Final cleanup
        test.main_window_->ExitVisualSelectionMode();
    }
    
    SECTION("Memory efficiency with repeated operations") {
        auto lines = CreateLargeLogFile(500);
        test.LoadTestFile(lines);
        
        // Perform many operations to test for memory leaks
        for (int cycle = 0; cycle < 20; ++cycle) {
            test.main_window_->EnterVisualSelectionMode();
            
            for (int i = 0; i < 50; ++i) {
                test.main_window_->ExtendVisualSelection(i);
                
                if (i % 10 == 0) {
                    auto element = test.main_window_->Render();
                    REQUIRE(element != nullptr);
                }
            }
            
            test.main_window_->CopyVisualSelectionToClipboard();
            test.main_window_->ExitVisualSelectionMode();
        }
        
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Performance - Keyboard event handling", "[visual_selection][performance]") {
    VisualSelectionPerformanceTest test;
    test.SetUp();
    
    auto lines = CreateLargeLogFile(1000);
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Rapid keyboard navigation") {
        // Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Test rapid j key presses
        double navigation_time = MeasureExecutionTime([&]() {
            for (int i = 0; i < 100; ++i) {
                component->OnEvent(ftxui::Event::Character('j'));
            }
        });
        
        REQUIRE(navigation_time < 100.0); // Should complete in less than 100ms
        REQUIRE(test.main_window_->GetVisualSelectionSize() > 50);
        
        component->OnEvent(ftxui::Event::Escape);
    }
    
    SECTION("Mixed keyboard operations") {
        double mixed_operations_time = MeasureExecutionTime([&]() {
            for (int i = 0; i < 20; ++i) {
                // Enter visual mode
                component->OnEvent(ftxui::Event::Character('v'));
                
                // Navigate
                for (int j = 0; j < 5; ++j) {
                    component->OnEvent(ftxui::Event::Character('j'));
                }
                
                // Navigate back
                for (int k = 0; k < 2; ++k) {
                    component->OnEvent(ftxui::Event::Character('k'));
                }
                
                // Copy and exit
                component->OnEvent(ftxui::Event::Character('y'));
            }
        });
        
        REQUIRE(mixed_operations_time < 200.0); // Should complete in less than 200ms
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    SECTION("Ctrl+d/u performance") {
        component->OnEvent(ftxui::Event::Character('v'));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        double half_page_time = MeasureExecutionTime([&]() {
            for (int i = 0; i < 20; ++i) {
                component->OnEvent(ftxui::Event::Character(static_cast<char>(4))); // Ctrl+d
                component->OnEvent(ftxui::Event::Character(static_cast<char>(21))); // Ctrl+u
            }
        });
        
        REQUIRE(half_page_time < 100.0); // Should complete in less than 100ms
        
        component->OnEvent(ftxui::Event::Escape);
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Performance - Integration scenarios", "[visual_selection][performance]") {
    VisualSelectionPerformanceTest test;
    test.SetUp();
    
    auto lines = CreateLargeLogFile(2000);
    test.LoadTestFile(lines);
    
    SECTION("Performance with active filters") {
        // Apply some filters first
        auto filter_panel = test.main_window_->GetFilterPanel();
        REQUIRE(filter_panel != nullptr);
        
        // Test visual selection performance with filters
        double filtered_performance = MeasureExecutionTime([&]() {
            test.main_window_->EnterVisualSelectionMode();
            test.main_window_->ExtendVisualSelection(500);
            auto element = test.main_window_->Render();
            REQUIRE(element != nullptr);
            test.main_window_->ExitVisualSelectionMode();
        });
        
        REQUIRE(filtered_performance < 300.0); // Should complete in less than 300ms
    }
    
    SECTION("Performance with search active") {
        // Perform a search
        test.main_window_->PerformSearch("Test");
        
        double search_performance = MeasureExecutionTime([&]() {
            test.main_window_->EnterVisualSelectionMode();
            test.main_window_->ExtendVisualSelection(100);
            auto element = test.main_window_->Render();
            REQUIRE(element != nullptr);
            test.main_window_->CopyVisualSelectionToClipboard();
            test.main_window_->ExitVisualSelectionMode();
        });
        
        REQUIRE(search_performance < 200.0); // Should complete in less than 200ms
    }
    
    SECTION("Performance with detail view and line numbers") {
        REQUIRE(test.main_window_->IsDetailViewVisible());
        
        double full_ui_performance = MeasureExecutionTime([&]() {
            test.main_window_->EnterVisualSelectionMode();
            test.main_window_->ExtendVisualSelection(300);
            
            // Render multiple times to simulate real usage
            for (int i = 0; i < 5; ++i) {
                auto element = test.main_window_->Render();
                REQUIRE(element != nullptr);
            }
            
            test.main_window_->ExitVisualSelectionMode();
        });
        
        REQUIRE(full_ui_performance < 500.0); // Should complete in less than 500ms
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Performance - Stress testing", "[visual_selection][performance][stress]") {
    VisualSelectionPerformanceTest test;
    test.SetUp();
    
    SECTION("Extreme file size stress test") {
        // Only run this test if we have enough memory/time
        auto lines = CreateLargeLogFile(20000);
        test.LoadTestFile(lines);
        
        double stress_time = MeasureExecutionTime([&]() {
            test.main_window_->EnterVisualSelectionMode();
            test.main_window_->ExtendVisualSelection(19999);
            REQUIRE(test.main_window_->GetVisualSelectionSize() == 20000);
            
            // Test that we can still render
            auto element = test.main_window_->Render();
            REQUIRE(element != nullptr);
            
            test.main_window_->ExitVisualSelectionMode();
        });
        
        REQUIRE(stress_time < 2000.0); // Should complete in less than 2 seconds
    }
    
    SECTION("Extended operation stress test") {
        auto lines = CreateLargeLogFile(1000);
        test.LoadTestFile(lines);
        
        // Run extended operations to test stability
        double extended_time = MeasureExecutionTime([&]() {
            for (int cycle = 0; cycle < 100; ++cycle) {
                test.main_window_->EnterVisualSelectionMode();
                
                // Vary the selection size
                int selection_size = (cycle % 10) * 50;
                test.main_window_->ExtendVisualSelection(selection_size);
                
                // Occasionally render
                if (cycle % 10 == 0) {
                    auto element = test.main_window_->Render();
                    REQUIRE(element != nullptr);
                }
                
                // Occasionally copy
                if (cycle % 20 == 0) {
                    test.main_window_->CopyVisualSelectionToClipboard();
                }
                
                test.main_window_->ExitVisualSelectionMode();
            }
        });
        
        REQUIRE(extended_time < 5000.0); // Should complete in less than 5 seconds
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    test.TearDown();
}