#include <catch2/catch_test_macros.hpp>
#include "../lib/config/config_manager.h"
#include <filesystem>
#include <fstream>

using namespace ue_log;

TEST_CASE("ColorScheme Tests", "[config][color_scheme]") {
    
    SECTION("Default constructor creates valid scheme") {
        ColorScheme scheme;
        REQUIRE(scheme.IsValid());
        REQUIRE(scheme.Get_name() == "Default");
        REQUIRE(scheme.Get_background_color() == "#000000");
        REQUIRE(scheme.Get_text_color() == "#FFFFFF");
    }
    
    SECTION("Named constructor sets name correctly") {
        ColorScheme scheme("TestScheme");
        REQUIRE(scheme.IsValid());
        REQUIRE(scheme.Get_name() == "TestScheme");
    }
    
    SECTION("Color validation works correctly") {
        ColorScheme scheme;
        
        // Valid colors
        scheme.Request_background_color("#FF0000");
        REQUIRE(scheme.IsValid());
        
        scheme.Request_background_color("#F00");
        REQUIRE(scheme.IsValid());
        
        // Invalid colors
        scheme.Request_background_color("invalid");
        REQUIRE_FALSE(scheme.IsValid());
        REQUIRE(scheme.GetValidationError().find("Invalid background color") != std::string::npos);
        
        scheme.Request_background_color("#GG0000");
        REQUIRE_FALSE(scheme.IsValid());
        
        scheme.Request_background_color("#FF00");
        REQUIRE_FALSE(scheme.IsValid());
    }
    
    SECTION("JSON serialization and deserialization") {
        ColorScheme original("TestScheme");
        original.Request_background_color("#1E1E1E");
        original.Request_text_color("#D4D4D4");
        original.Request_error_color("#F44747");
        
        std::string json = original.ToJson();
        REQUIRE_FALSE(json.empty());
        
        auto deserialized = ColorScheme::FromJson(json);
        REQUIRE(deserialized != nullptr);
        REQUIRE(deserialized->Get_name() == "TestScheme");
        REQUIRE(deserialized->Get_background_color() == "#1E1E1E");
        REQUIRE(deserialized->Get_text_color() == "#D4D4D4");
        REQUIRE(deserialized->Get_error_color() == "#F44747");
        REQUIRE(deserialized->IsValid());
    }
    
    SECTION("Predefined schemes are valid") {
        auto dark = ColorScheme::GetDefaultDarkScheme();
        REQUIRE(dark.IsValid());
        REQUIRE(dark.Get_name() == "Dark");
        
        auto light = ColorScheme::GetDefaultLightScheme();
        REQUIRE(light.IsValid());
        REQUIRE(light.Get_name() == "Light");
    }
}

TEST_CASE("KeyBindings Tests", "[config][key_bindings]") {
    
    SECTION("Default constructor creates valid bindings") {
        KeyBindings bindings;
        REQUIRE(bindings.IsValid());
        REQUIRE(bindings.HasKeyBinding("scroll_up"));
        REQUIRE(bindings.GetKeyBinding("scroll_up") == "Up");
    }
    
    SECTION("Key binding management") {
        KeyBindings bindings;
        
        // Set new binding
        bindings.SetKeyBinding("test_action", "Ctrl+T");
        REQUIRE(bindings.HasKeyBinding("test_action"));
        REQUIRE(bindings.GetKeyBinding("test_action") == "Ctrl+T");
        
        // Remove binding
        bindings.RemoveKeyBinding("test_action");
        REQUIRE_FALSE(bindings.HasKeyBinding("test_action"));
        REQUIRE(bindings.GetKeyBinding("test_action").empty());
        
        // Clear all bindings
        bindings.ClearAllBindings();
        REQUIRE_FALSE(bindings.HasKeyBinding("scroll_up"));
    }
    
    SECTION("Validation") {
        KeyBindings bindings;
        bindings.ClearAllBindings();
        
        // Valid action and key
        bindings.SetKeyBinding("valid_action", "Ctrl+V");
        REQUIRE(bindings.IsValid());
        
        // Invalid action (manually add to bypass validation)
        auto& key_mappings = bindings.Get_key_mappings();
        const_cast<std::unordered_map<std::string, std::string>&>(key_mappings)["123invalid"] = "Ctrl+I";
        REQUIRE_FALSE(bindings.IsValid());
        REQUIRE(bindings.GetValidationError().find("Invalid action") != std::string::npos);
    }
    
    SECTION("JSON serialization and deserialization") {
        KeyBindings original;
        original.ClearAllBindings();
        original.SetKeyBinding("action1", "Ctrl+1");
        original.SetKeyBinding("action2", "Ctrl+2");
        
        std::string json = original.ToJson();
        REQUIRE_FALSE(json.empty());
        
        auto deserialized = KeyBindings::FromJson(json);
        REQUIRE(deserialized != nullptr);
        REQUIRE(deserialized->HasKeyBinding("action1"));
        REQUIRE(deserialized->GetKeyBinding("action1") == "Ctrl+1");
        REQUIRE(deserialized->HasKeyBinding("action2"));
        REQUIRE(deserialized->GetKeyBinding("action2") == "Ctrl+2");
    }
    
    SECTION("Default key bindings are comprehensive") {
        auto defaults = KeyBindings::GetDefaultKeyBindings();
        REQUIRE(defaults.IsValid());
        
        // Check essential bindings exist
        REQUIRE(defaults.HasKeyBinding("open_file"));
        REQUIRE(defaults.HasKeyBinding("quit"));
        REQUIRE(defaults.HasKeyBinding("search"));
        REQUIRE(defaults.HasKeyBinding("scroll_up"));
        REQUIRE(defaults.HasKeyBinding("scroll_down"));
    }
}

TEST_CASE("AppConfig Tests", "[config][app_config]") {
    
    SECTION("Default constructor creates valid config") {
        AppConfig config;
        REQUIRE(config.IsValid());
        REQUIRE(config.Get_version() == "1.0");
        REQUIRE(config.Get_max_recent_files() == 10);
        REQUIRE(config.Get_auto_save_config() == true);
        REQUIRE(config.Get_enable_real_time_monitoring() == true);
    }
    
    SECTION("Recent files management") {
        AppConfig config;
        
        // Add files
        config.AddRecentFile("/path/to/file1.log");
        config.AddRecentFile("/path/to/file2.log");
        REQUIRE(config.Get_recent_files().size() == 2);
        REQUIRE(config.Get_recent_files()[0] == "/path/to/file2.log"); // Most recent first
        REQUIRE(config.Get_recent_files()[1] == "/path/to/file1.log");
        
        // Add duplicate (should move to front)
        config.AddRecentFile("/path/to/file1.log");
        REQUIRE(config.Get_recent_files().size() == 2);
        REQUIRE(config.Get_recent_files()[0] == "/path/to/file1.log");
        
        // Remove file
        config.RemoveRecentFile("/path/to/file1.log");
        REQUIRE(config.Get_recent_files().size() == 1);
        REQUIRE(config.Get_recent_files()[0] == "/path/to/file2.log");
        
        // Clear all
        config.ClearRecentFiles();
        REQUIRE(config.Get_recent_files().empty());
    }
    
    SECTION("Recent files limit enforcement") {
        AppConfig config;
        config.Request_max_recent_files(3);
        
        // Add more files than limit
        for (int i = 1; i <= 5; ++i) {
            config.AddRecentFile("/path/to/file" + std::to_string(i) + ".log");
        }
        
        REQUIRE(config.Get_recent_files().size() == 3);
        REQUIRE(config.Get_recent_files()[0] == "/path/to/file5.log");
        REQUIRE(config.Get_recent_files()[1] == "/path/to/file4.log");
        REQUIRE(config.Get_recent_files()[2] == "/path/to/file3.log");
    }
    
    SECTION("Validation") {
        AppConfig config;
        
        // Valid config
        REQUIRE(config.IsValid());
        
        // Invalid version
        config.Request_version("");
        REQUIRE_FALSE(config.IsValid());
        REQUIRE(config.GetValidationError().find("Version cannot be empty") != std::string::npos);
        
        config.Request_version("1.0");
        REQUIRE(config.IsValid());
        
        // Invalid max recent files
        config.Request_max_recent_files(-1);
        REQUIRE_FALSE(config.IsValid());
        
        config.Request_max_recent_files(100);
        REQUIRE_FALSE(config.IsValid());
        
        config.Request_max_recent_files(10);
        REQUIRE(config.IsValid());
        
        // Invalid poll interval
        config.Request_file_monitor_poll_interval_ms(5);
        REQUIRE_FALSE(config.IsValid());
        
        config.Request_file_monitor_poll_interval_ms(15000);
        REQUIRE_FALSE(config.IsValid());
        
        config.Request_file_monitor_poll_interval_ms(100);
        REQUIRE(config.IsValid());
    }
    
    SECTION("JSON serialization and deserialization") {
        AppConfig original;
        original.Request_version("1.0");
        original.Request_last_opened_file("/test/file.log");
        original.AddRecentFile("/recent1.log");
        original.AddRecentFile("/recent2.log");
        original.Request_max_recent_files(5);
        original.Request_auto_save_config(false);
        
        std::string json = original.ToJson();
        REQUIRE_FALSE(json.empty());
        
        AppConfig deserialized;
        Result result = deserialized.FromJson(json);
        REQUIRE(result.IsSuccess());
        
        REQUIRE(deserialized.Get_version() == "1.0");
        REQUIRE(deserialized.Get_last_opened_file() == "/test/file.log");
        REQUIRE(deserialized.Get_recent_files().size() == 2);
        REQUIRE(deserialized.Get_recent_files()[0] == "/recent2.log");
        REQUIRE(deserialized.Get_recent_files()[1] == "/recent1.log");
        REQUIRE(deserialized.Get_max_recent_files() == 5);
        REQUIRE(deserialized.Get_auto_save_config() == false);
        REQUIRE(deserialized.IsValid());
    }
    
    SECTION("File operations") {
        std::string test_config_path = "test_config.json";
        
        // Clean up any existing test file
        if (std::filesystem::exists(test_config_path)) {
            std::filesystem::remove(test_config_path);
        }
        
        AppConfig config;
        config.Request_last_opened_file("/test/file.log");
        config.AddRecentFile("/recent.log");
        
        // Save to file
        Result save_result = config.SaveToFile(test_config_path);
        REQUIRE(save_result.IsSuccess());
        REQUIRE(std::filesystem::exists(test_config_path));
        
        // Load from file
        AppConfig loaded_config;
        Result load_result = loaded_config.LoadFromFile(test_config_path);
        REQUIRE(load_result.IsSuccess());
        REQUIRE(loaded_config.Get_last_opened_file() == "/test/file.log");
        REQUIRE(loaded_config.Get_recent_files().size() == 1);
        REQUIRE(loaded_config.Get_recent_files()[0] == "/recent.log");
        
        // Clean up
        std::filesystem::remove(test_config_path);
    }
    
    SECTION("Default config is valid") {
        auto default_config = AppConfig::GetDefaultConfig();
        REQUIRE(default_config.IsValid());
        REQUIRE(default_config.Get_version() == "1.0");
        REQUIRE(default_config.Get_color_scheme().IsValid());
        REQUIRE(default_config.Get_key_bindings().IsValid());
    }
}

TEST_CASE("ConfigManager Tests", "[config][config_manager]") {
    
    SECTION("Default constructor") {
        ConfigManager manager;
        REQUIRE_FALSE(manager.Get_config_loaded());
        REQUIRE(manager.IsConfigValid());
        REQUIRE(manager.GetConfig().Get_version() == "1.0");
    }
    
    SECTION("Constructor with path") {
        ConfigManager manager("test_path.json");
        REQUIRE(manager.Get_config_file_path() == "test_path.json");
        REQUIRE_FALSE(manager.Get_config_loaded());
    }
    
    SECTION("Config access and modification") {
        ConfigManager manager;
        
        // Get config reference and modify
        AppConfig& config = manager.GetConfig();
        config.Request_last_opened_file("/modified/file.log");
        
        REQUIRE(manager.GetConfig().Get_last_opened_file() == "/modified/file.log");
        
        // Set new config
        AppConfig new_config;
        new_config.Request_last_opened_file("/new/file.log");
        manager.SetConfig(new_config);
        
        REQUIRE(manager.GetConfig().Get_last_opened_file() == "/new/file.log");
    }
    
    SECTION("Reset to defaults") {
        ConfigManager manager;
        
        // Modify config
        manager.GetConfig().Request_last_opened_file("/modified.log");
        manager.GetConfig().Request_max_recent_files(20);
        
        // Reset
        Result result = manager.ResetToDefaults();
        REQUIRE(result.IsSuccess());
        REQUIRE(manager.Get_config_loaded());
        REQUIRE(manager.GetConfig().Get_last_opened_file().empty());
        REQUIRE(manager.GetConfig().Get_max_recent_files() == 10);
    }
    
    SECTION("File operations") {
        std::string test_config_path = "test_manager_config.json";
        
        // Clean up any existing test file
        if (std::filesystem::exists(test_config_path)) {
            std::filesystem::remove(test_config_path);
        }
        
        ConfigManager manager(test_config_path);
        
        // Modify config
        manager.GetConfig().Request_last_opened_file("/manager/test.log");
        manager.GetConfig().AddRecentFile("/manager/recent.log");
        
        // Save config
        Result save_result = manager.SaveConfig();
        REQUIRE(save_result.IsSuccess());
        REQUIRE(std::filesystem::exists(test_config_path));
        
        // Create new manager and load
        ConfigManager new_manager;
        Result load_result = new_manager.LoadConfig(test_config_path);
        REQUIRE(load_result.IsSuccess());
        REQUIRE(new_manager.Get_config_loaded());
        REQUIRE(new_manager.GetConfig().Get_last_opened_file() == "/manager/test.log");
        REQUIRE(new_manager.GetConfig().Get_recent_files().size() == 1);
        REQUIRE(new_manager.GetConfig().Get_recent_files()[0] == "/manager/recent.log");
        
        // Clean up
        std::filesystem::remove(test_config_path);
    }
    
    SECTION("Load non-existent config creates default") {
        std::string non_existent_path = "non_existent_config.json";
        
        // Ensure file doesn't exist
        if (std::filesystem::exists(non_existent_path)) {
            std::filesystem::remove(non_existent_path);
        }
        
        ConfigManager manager;
        Result result = manager.LoadConfig(non_existent_path);
        REQUIRE(result.IsSuccess());
        REQUIRE(manager.Get_config_loaded());
        REQUIRE(std::filesystem::exists(non_existent_path));
        
        // Verify it's a default config
        REQUIRE(manager.GetConfig().Get_version() == "1.0");
        REQUIRE(manager.GetConfig().Get_max_recent_files() == 10);
        
        // Clean up
        std::filesystem::remove(non_existent_path);
    }
    
    SECTION("Default config path generation") {
        ConfigManager manager;
        std::string default_path = manager.GetDefaultConfigPath();
        REQUIRE_FALSE(default_path.empty());
        
        #ifdef _WIN32
            REQUIRE(default_path.find(".unreal_log_viewer") != std::string::npos);
            REQUIRE(default_path.find("config.json") != std::string::npos);
        #else
            REQUIRE(default_path.find(".config/unreal_log_viewer") != std::string::npos);
            REQUIRE(default_path.find("config.json") != std::string::npos);
        #endif
    }
    
    SECTION("Config file existence checking") {
        ConfigManager manager;
        
        // Non-existent file
        REQUIRE_FALSE(manager.ConfigFileExists("non_existent.json"));
        
        // Create a test file
        std::string test_file = "existence_test.json";
        std::ofstream file(test_file);
        file << "{}";
        file.close();
        
        REQUIRE(manager.ConfigFileExists(test_file));
        
        // Clean up
        std::filesystem::remove(test_file);
    }
    
    SECTION("Validation methods") {
        ConfigManager manager;
        
        REQUIRE(manager.IsConfigValid());
        REQUIRE(manager.GetConfigValidationError().empty());
        
        // Make config invalid
        manager.GetConfig().Request_version("");
        
        REQUIRE_FALSE(manager.IsConfigValid());
        REQUIRE_FALSE(manager.GetConfigValidationError().empty());
    }
}

TEST_CASE("Configuration Integration Tests", "[config][integration]") {
    
    SECTION("Complete configuration workflow") {
        std::string config_path = "integration_test_config.json";
        
        // Clean up
        if (std::filesystem::exists(config_path)) {
            std::filesystem::remove(config_path);
        }
        
        // Create and configure manager
        ConfigManager manager(config_path);
        
        // Customize configuration
        manager.GetConfig().Request_last_opened_file("/integration/test.log");
        manager.GetConfig().AddRecentFile("/integration/recent1.log");
        manager.GetConfig().AddRecentFile("/integration/recent2.log");
        manager.GetConfig().Request_enable_real_time_monitoring(false);
        manager.GetConfig().Request_max_log_entries(50000);
        
        // Customize color scheme
        ColorScheme custom_scheme("Custom");
        custom_scheme.Request_background_color("#2D2D30");
        custom_scheme.Request_text_color("#F1F1F1");
        manager.GetConfig().Request_color_scheme(custom_scheme);
        
        // Customize key bindings
        KeyBindings custom_bindings;
        custom_bindings.ClearAllBindings();
        custom_bindings.SetKeyBinding("custom_action", "Ctrl+Shift+X");
        manager.GetConfig().Request_key_bindings(custom_bindings);
        
        // Save configuration
        Result save_result = manager.SaveConfig();
        REQUIRE(save_result.IsSuccess());
        
        // Load in new manager
        ConfigManager new_manager;
        Result load_result = new_manager.LoadConfig(config_path);
        REQUIRE(load_result.IsSuccess());
        
        // Verify all settings preserved
        const AppConfig& loaded_config = new_manager.GetConfig();
        REQUIRE(loaded_config.Get_last_opened_file() == "/integration/test.log");
        REQUIRE(loaded_config.Get_recent_files().size() == 2);
        REQUIRE(loaded_config.Get_recent_files()[0] == "/integration/recent2.log");
        REQUIRE(loaded_config.Get_recent_files()[1] == "/integration/recent1.log");
        REQUIRE(loaded_config.Get_enable_real_time_monitoring() == false);
        REQUIRE(loaded_config.Get_max_log_entries() == 50000);
        
        // Verify color scheme
        const ColorScheme& loaded_scheme = loaded_config.Get_color_scheme();
        REQUIRE(loaded_scheme.Get_name() == "Custom");
        REQUIRE(loaded_scheme.Get_background_color() == "#2D2D30");
        REQUIRE(loaded_scheme.Get_text_color() == "#F1F1F1");
        
        // Verify key bindings
        const KeyBindings& loaded_bindings = loaded_config.Get_key_bindings();
        REQUIRE(loaded_bindings.HasKeyBinding("custom_action"));
        REQUIRE(loaded_bindings.GetKeyBinding("custom_action") == "Ctrl+Shift+X");
        
        // Clean up
        std::filesystem::remove(config_path);
    }
}