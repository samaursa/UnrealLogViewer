#pragma once

#include "../../macros.h"
#include "../common/result.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace ue_log {
    
    // Color scheme configuration
    class ColorScheme {
        CK_GENERATED_BODY(ColorScheme);
        
    private:
        std::string name;
        std::string background_color;
        std::string text_color;
        std::string highlight_color;
        std::string error_color;
        std::string warning_color;
        std::string info_color;
        std::string debug_color;
        std::string selection_color;
        std::string border_color;
        
    public:
        // Default constructor
        ColorScheme();
        
        // Constructor with name
        ColorScheme(const std::string& scheme_name);
        
        // Properties using macros
        CK_PROPERTY(name);
        CK_PROPERTY(background_color);
        CK_PROPERTY(text_color);
        CK_PROPERTY(highlight_color);
        CK_PROPERTY(error_color);
        CK_PROPERTY(warning_color);
        CK_PROPERTY(info_color);
        CK_PROPERTY(debug_color);
        CK_PROPERTY(selection_color);
        CK_PROPERTY(border_color);
        
        // Validation
        bool IsValid() const;
        std::string GetValidationError() const;
        
        // Serialization
        std::string ToJson() const;
        static std::unique_ptr<ColorScheme> FromJson(const std::string& json_data);
        
        // Predefined color schemes
        static ColorScheme GetDefaultDarkScheme();
        static ColorScheme GetDefaultLightScheme();
        
    private:
        bool IsValidColor(const std::string& color) const;
        std::string EscapeJsonString(const std::string& str) const;
        static std::string UnescapeJsonString(const std::string& str);
    };
    
    // Key bindings configuration
    class KeyBindings {
        CK_GENERATED_BODY(KeyBindings);
        
    private:
        std::unordered_map<std::string, std::string> key_mappings;
        
    public:
        // Default constructor
        KeyBindings();
        
        // Properties using macros
        CK_PROPERTY_GET(key_mappings);
        
        // Key binding management
        void SetKeyBinding(const std::string& action, const std::string& key);
        std::string GetKeyBinding(const std::string& action) const;
        bool HasKeyBinding(const std::string& action) const;
        void RemoveKeyBinding(const std::string& action);
        void ClearAllBindings();
        
        // Validation
        bool IsValid() const;
        std::string GetValidationError() const;
        
        // Serialization
        std::string ToJson() const;
        static std::unique_ptr<KeyBindings> FromJson(const std::string& json_data);
        
        // Default key bindings
        static KeyBindings GetDefaultKeyBindings();
        
    private:
        bool IsValidKey(const std::string& key) const;
        bool IsValidAction(const std::string& action) const;
        std::string EscapeJsonString(const std::string& str) const;
        static std::string UnescapeJsonString(const std::string& str);
    };
    
    // Main application configuration
    class AppConfig {
        CK_GENERATED_BODY(AppConfig);
        
    private:
        std::string version;
        ColorScheme color_scheme;
        KeyBindings key_bindings;
        std::string last_opened_file;
        std::vector<std::string> recent_files;
        int max_recent_files;
        bool auto_save_config;
        bool enable_real_time_monitoring;
        int file_monitor_poll_interval_ms;
        bool enable_syntax_highlighting;
        bool show_line_numbers;
        bool word_wrap;
        int max_log_entries;
        bool auto_scroll_to_bottom;
        std::string default_log_level_filter;
        
    public:
        // Default constructor
        AppConfig();
        
        // Properties using macros
        CK_PROPERTY(version);
        CK_PROPERTY(color_scheme);
        CK_PROPERTY(key_bindings);
        CK_PROPERTY(last_opened_file);
        CK_PROPERTY(recent_files);
        CK_PROPERTY(max_recent_files);
        CK_PROPERTY(auto_save_config);
        CK_PROPERTY(enable_real_time_monitoring);
        CK_PROPERTY(file_monitor_poll_interval_ms);
        CK_PROPERTY(enable_syntax_highlighting);
        CK_PROPERTY(show_line_numbers);
        CK_PROPERTY(word_wrap);
        CK_PROPERTY(max_log_entries);
        CK_PROPERTY(auto_scroll_to_bottom);
        CK_PROPERTY(default_log_level_filter);
        
        // Recent files management
        void AddRecentFile(const std::string& file_path);
        void RemoveRecentFile(const std::string& file_path);
        void ClearRecentFiles();
        
        // Validation
        bool IsValid() const;
        std::string GetValidationError() const;
        
        // File operations
        Result LoadFromFile(const std::string& config_file_path);
        Result SaveToFile(const std::string& config_file_path) const;
        
        // Serialization
        std::string ToJson() const;
        Result FromJson(const std::string& json_data);
        
        // Default configuration
        static AppConfig GetDefaultConfig();
        
        // Configuration migration
        Result MigrateFromVersion(const std::string& from_version);
        
    private:
        void ApplyDefaults();
        std::string EscapeJsonString(const std::string& str) const;
        static std::string UnescapeJsonString(const std::string& str);
    };
    
    // Configuration manager
    class ConfigManager {
        CK_GENERATED_BODY(ConfigManager);
        
    private:
        AppConfig current_config;
        std::string config_file_path;
        bool config_loaded;
        
    public:
        // Default constructor
        ConfigManager();
        
        // Constructor with config file path
        ConfigManager(const std::string& config_path);
        
        // Properties using macros
        CK_PROPERTY_GET(current_config);
        CK_PROPERTY_GET(config_file_path);
        CK_PROPERTY_GET(config_loaded);
        
        // Configuration management
        Result LoadConfig();
        Result LoadConfig(const std::string& config_path);
        Result SaveConfig();
        Result SaveConfig(const std::string& config_path);
        Result ResetToDefaults();
        
        // Configuration access
        AppConfig& GetConfig();
        const AppConfig& GetConfig() const;
        void SetConfig(const AppConfig& config);
        
        // Validation
        bool IsConfigValid() const;
        std::string GetConfigValidationError() const;
        
        // Utility methods
        std::string GetDefaultConfigPath() const;
        bool ConfigFileExists() const;
        bool ConfigFileExists(const std::string& config_path) const;
        
    private:
        void SetConfigPath(const std::string& config_path);
    };
    
} // namespace ue_log