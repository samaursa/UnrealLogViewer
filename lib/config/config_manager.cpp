#include "config_manager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <regex>

namespace ue_log {
    
    // ColorScheme implementation
    
    ColorScheme::ColorScheme() 
        : name("Default"), 
          background_color("#000000"),
          text_color("#FFFFFF"),
          highlight_color("#FFFF00"),
          error_color("#FF0000"),
          warning_color("#FFA500"),
          info_color("#00FF00"),
          debug_color("#808080"),
          selection_color("#0080FF"),
          border_color("#808080") {
    }
    
    ColorScheme::ColorScheme(const std::string& scheme_name) 
        : ColorScheme() {
        name = scheme_name;
    }
    
    bool ColorScheme::IsValid() const {
        if (name.empty()) return false;
        
        return IsValidColor(background_color) &&
               IsValidColor(text_color) &&
               IsValidColor(highlight_color) &&
               IsValidColor(error_color) &&
               IsValidColor(warning_color) &&
               IsValidColor(info_color) &&
               IsValidColor(debug_color) &&
               IsValidColor(selection_color) &&
               IsValidColor(border_color);
    }
    
    std::string ColorScheme::GetValidationError() const {
        if (name.empty()) {
            return "Color scheme name cannot be empty";
        }
        
        if (!IsValidColor(background_color)) return "Invalid background color: " + background_color;
        if (!IsValidColor(text_color)) return "Invalid text color: " + text_color;
        if (!IsValidColor(highlight_color)) return "Invalid highlight color: " + highlight_color;
        if (!IsValidColor(error_color)) return "Invalid error color: " + error_color;
        if (!IsValidColor(warning_color)) return "Invalid warning color: " + warning_color;
        if (!IsValidColor(info_color)) return "Invalid info color: " + info_color;
        if (!IsValidColor(debug_color)) return "Invalid debug color: " + debug_color;
        if (!IsValidColor(selection_color)) return "Invalid selection color: " + selection_color;
        if (!IsValidColor(border_color)) return "Invalid border color: " + border_color;
        
        return "";
    }
    
    std::string ColorScheme::ToJson() const {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"name\": \"" << EscapeJsonString(name) << "\",\n";
        oss << "  \"background_color\": \"" << EscapeJsonString(background_color) << "\",\n";
        oss << "  \"text_color\": \"" << EscapeJsonString(text_color) << "\",\n";
        oss << "  \"highlight_color\": \"" << EscapeJsonString(highlight_color) << "\",\n";
        oss << "  \"error_color\": \"" << EscapeJsonString(error_color) << "\",\n";
        oss << "  \"warning_color\": \"" << EscapeJsonString(warning_color) << "\",\n";
        oss << "  \"info_color\": \"" << EscapeJsonString(info_color) << "\",\n";
        oss << "  \"debug_color\": \"" << EscapeJsonString(debug_color) << "\",\n";
        oss << "  \"selection_color\": \"" << EscapeJsonString(selection_color) << "\",\n";
        oss << "  \"border_color\": \"" << EscapeJsonString(border_color) << "\"\n";
        oss << "}";
        return oss.str();
    }
    
    std::unique_ptr<ColorScheme> ColorScheme::FromJson(const std::string& json_data) {
        auto scheme = std::make_unique<ColorScheme>();
        
        // Simple JSON parsing - extract values
        auto extract_value = [&](const std::string& key) -> std::string {
            std::string search_key = "\"" + key + "\": \"";
            size_t start = json_data.find(search_key);
            if (start == std::string::npos) return "";
            start += search_key.length();
            size_t end = json_data.find("\"", start);
            if (end == std::string::npos) return "";
            return UnescapeJsonString(json_data.substr(start, end - start));
        };
        
        scheme->Request_name(extract_value("name"));
        scheme->Request_background_color(extract_value("background_color"));
        scheme->Request_text_color(extract_value("text_color"));
        scheme->Request_highlight_color(extract_value("highlight_color"));
        scheme->Request_error_color(extract_value("error_color"));
        scheme->Request_warning_color(extract_value("warning_color"));
        scheme->Request_info_color(extract_value("info_color"));
        scheme->Request_debug_color(extract_value("debug_color"));
        scheme->Request_selection_color(extract_value("selection_color"));
        scheme->Request_border_color(extract_value("border_color"));
        
        return scheme;
    }
    
    ColorScheme ColorScheme::GetDefaultDarkScheme() {
        ColorScheme scheme("Dark");
        scheme.Request_background_color("#1E1E1E");
        scheme.Request_text_color("#D4D4D4");
        scheme.Request_highlight_color("#FFFF00");
        scheme.Request_error_color("#F44747");
        scheme.Request_warning_color("#FF8C00");
        scheme.Request_info_color("#4EC9B0");
        scheme.Request_debug_color("#9CDCFE");
        scheme.Request_selection_color("#264F78");
        scheme.Request_border_color("#3C3C3C");
        return scheme;
    }
    
    ColorScheme ColorScheme::GetDefaultLightScheme() {
        ColorScheme scheme("Light");
        scheme.Request_background_color("#FFFFFF");
        scheme.Request_text_color("#000000");
        scheme.Request_highlight_color("#FFFF00");
        scheme.Request_error_color("#CD3131");
        scheme.Request_warning_color("#FF8C00");
        scheme.Request_info_color("#008000");
        scheme.Request_debug_color("#0000FF");
        scheme.Request_selection_color("#ADD6FF");
        scheme.Request_border_color("#CCCCCC");
        return scheme;
    }
    
    bool ColorScheme::IsValidColor(const std::string& color) const {
        if (color.empty()) return false;
        
        // Check for hex color format (#RRGGBB or #RGB)
        std::regex hex_pattern("^#([0-9A-Fa-f]{3}|[0-9A-Fa-f]{6})$");
        return std::regex_match(color, hex_pattern);
    }
    
    std::string ColorScheme::EscapeJsonString(const std::string& str) const {
        std::string escaped;
        for (char c : str) {
            switch (c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }
    
    std::string ColorScheme::UnescapeJsonString(const std::string& str) {
        std::string unescaped;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[i + 1]) {
                    case '"': unescaped += '"'; i++; break;
                    case '\\': unescaped += '\\'; i++; break;
                    case 'n': unescaped += '\n'; i++; break;
                    case 'r': unescaped += '\r'; i++; break;
                    case 't': unescaped += '\t'; i++; break;
                    default: unescaped += str[i]; break;
                }
            } else {
                unescaped += str[i];
            }
        }
        return unescaped;
    } 
   
    // KeyBindings implementation
    
    KeyBindings::KeyBindings() {
        // Initialize with default key bindings
        // Navigation
        SetKeyBinding("scroll_up", "Up");
        SetKeyBinding("scroll_down", "Down");
        SetKeyBinding("page_up", "PageUp");
        SetKeyBinding("page_down", "PageDown");
        SetKeyBinding("home", "Home");
        SetKeyBinding("end", "End");
        
        // File operations
        SetKeyBinding("open_file", "Ctrl+O");
        SetKeyBinding("reload_file", "F5");
        SetKeyBinding("quit", "Ctrl+Q");
        
        // Search and filter
        SetKeyBinding("search", "Ctrl+F");
        SetKeyBinding("filter", "Ctrl+Shift+F");
        SetKeyBinding("clear_filters", "Ctrl+Shift+C");
        
        // View
        SetKeyBinding("toggle_line_numbers", "Ctrl+L");
        SetKeyBinding("toggle_word_wrap", "Ctrl+W");
        SetKeyBinding("toggle_real_time", "Ctrl+R");
    }
    
    void KeyBindings::SetKeyBinding(const std::string& action, const std::string& key) {
        if (IsValidAction(action) && IsValidKey(key)) {
            key_mappings[action] = key;
        }
    }
    
    std::string KeyBindings::GetKeyBinding(const std::string& action) const {
        auto it = key_mappings.find(action);
        return (it != key_mappings.end()) ? it->second : "";
    }
    
    bool KeyBindings::HasKeyBinding(const std::string& action) const {
        return key_mappings.find(action) != key_mappings.end();
    }
    
    void KeyBindings::RemoveKeyBinding(const std::string& action) {
        key_mappings.erase(action);
    }
    
    void KeyBindings::ClearAllBindings() {
        key_mappings.clear();
    }
    
    bool KeyBindings::IsValid() const {
        for (const auto& pair : key_mappings) {
            if (!IsValidAction(pair.first) || !IsValidKey(pair.second)) {
                return false;
            }
        }
        return true;
    }
    
    std::string KeyBindings::GetValidationError() const {
        for (const auto& pair : key_mappings) {
            if (!IsValidAction(pair.first)) {
                return "Invalid action: " + pair.first;
            }
            if (!IsValidKey(pair.second)) {
                return "Invalid key: " + pair.second;
            }
        }
        return "";
    }
    
    std::string KeyBindings::ToJson() const {
        std::ostringstream oss;
        oss << "{\n";
        
        bool first = true;
        for (const auto& pair : key_mappings) {
            if (!first) oss << ",\n";
            oss << "  \"" << EscapeJsonString(pair.first) << "\": \"" << EscapeJsonString(pair.second) << "\"";
            first = false;
        }
        
        oss << "\n}";
        return oss.str();
    }
    
    std::unique_ptr<KeyBindings> KeyBindings::FromJson(const std::string& json_data) {
        auto bindings = std::make_unique<KeyBindings>();
        bindings->ClearAllBindings();
        
        // Simple JSON parsing for key-value pairs
        std::regex pair_pattern("\"([^\"]+)\"\\s*:\\s*\"([^\"]+)\"");
        std::sregex_iterator iter(json_data.begin(), json_data.end(), pair_pattern);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            std::string action = UnescapeJsonString((*iter)[1].str());
            std::string key = UnescapeJsonString((*iter)[2].str());
            bindings->SetKeyBinding(action, key);
        }
        
        return bindings;
    }
    
    KeyBindings KeyBindings::GetDefaultKeyBindings() {
        KeyBindings bindings;
        bindings.ClearAllBindings();
        
        // Navigation
        bindings.SetKeyBinding("scroll_up", "Up");
        bindings.SetKeyBinding("scroll_down", "Down");
        bindings.SetKeyBinding("page_up", "PageUp");
        bindings.SetKeyBinding("page_down", "PageDown");
        bindings.SetKeyBinding("home", "Home");
        bindings.SetKeyBinding("end", "End");
        
        // File operations
        bindings.SetKeyBinding("open_file", "Ctrl+O");
        bindings.SetKeyBinding("reload_file", "F5");
        bindings.SetKeyBinding("quit", "Ctrl+Q");
        
        // Search and filter
        bindings.SetKeyBinding("search", "Ctrl+F");
        bindings.SetKeyBinding("filter", "Ctrl+Shift+F");
        bindings.SetKeyBinding("clear_filters", "Ctrl+Shift+C");
        
        // View
        bindings.SetKeyBinding("toggle_line_numbers", "Ctrl+L");
        bindings.SetKeyBinding("toggle_word_wrap", "Ctrl+W");
        bindings.SetKeyBinding("toggle_real_time", "Ctrl+R");
        
        return bindings;
    }
    
    bool KeyBindings::IsValidKey(const std::string& key) const {
        if (key.empty()) return false;
        
        // Basic validation - key should not be empty and should contain valid characters
        // This is a simplified validation - in a real implementation, you'd validate against
        // the actual key codes supported by your UI framework
        return key.length() > 0 && key.length() < 50;
    }
    
    bool KeyBindings::IsValidAction(const std::string& action) const {
        if (action.empty()) return false;
        
        // Valid actions should be non-empty and contain only alphanumeric characters and underscores
        std::regex action_pattern("^[a-zA-Z_][a-zA-Z0-9_]*$");
        return std::regex_match(action, action_pattern);
    }
    
    std::string KeyBindings::EscapeJsonString(const std::string& str) const {
        std::string escaped;
        for (char c : str) {
            switch (c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }
    
    std::string KeyBindings::UnescapeJsonString(const std::string& str) {
        std::string unescaped;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[i + 1]) {
                    case '"': unescaped += '"'; i++; break;
                    case '\\': unescaped += '\\'; i++; break;
                    case 'n': unescaped += '\n'; i++; break;
                    case 'r': unescaped += '\r'; i++; break;
                    case 't': unescaped += '\t'; i++; break;
                    default: unescaped += str[i]; break;
                }
            } else {
                unescaped += str[i];
            }
        }
        return unescaped;
    }    

    // AppConfig implementation
    
    AppConfig::AppConfig() {
        ApplyDefaults();
    }
    
    void AppConfig::AddRecentFile(const std::string& file_path) {
        if (file_path.empty()) return;
        
        // Remove if already exists
        RemoveRecentFile(file_path);
        
        // Add to front
        recent_files.insert(recent_files.begin(), file_path);
        
        // Trim to max size
        if (static_cast<int>(recent_files.size()) > max_recent_files) {
            recent_files.resize(max_recent_files);
        }
    }
    
    void AppConfig::RemoveRecentFile(const std::string& file_path) {
        recent_files.erase(
            std::remove(recent_files.begin(), recent_files.end(), file_path),
            recent_files.end()
        );
    }
    
    void AppConfig::ClearRecentFiles() {
        recent_files.clear();
    }
    
    bool AppConfig::IsValid() const {
        if (version.empty()) return false;
        if (max_recent_files < 0 || max_recent_files > 50) return false;
        if (file_monitor_poll_interval_ms < 10 || file_monitor_poll_interval_ms > 10000) return false;
        if (max_log_entries < 100 || max_log_entries > 10000000) return false;
        
        return color_scheme.IsValid() && key_bindings.IsValid();
    }
    
    std::string AppConfig::GetValidationError() const {
        if (version.empty()) return "Version cannot be empty";
        if (max_recent_files < 0 || max_recent_files > 50) return "Max recent files must be between 0 and 50";
        if (file_monitor_poll_interval_ms < 10 || file_monitor_poll_interval_ms > 10000) return "Poll interval must be between 10ms and 10s";
        if (max_log_entries < 100 || max_log_entries > 10000000) return "Max log entries must be between 100 and 10M";
        
        if (!color_scheme.IsValid()) return "Color scheme is invalid: " + color_scheme.GetValidationError();
        if (!key_bindings.IsValid()) return "Key bindings are invalid: " + key_bindings.GetValidationError();
        
        return "";
    }
    
    Result AppConfig::LoadFromFile(const std::string& config_file_path) {
        try {
            std::ifstream file(config_file_path);
            if (!file.is_open()) {
                return Result::Error(ErrorCode::FileNotFound, 
                    "Cannot open config file: " + config_file_path);
            }
            
            std::string json_data((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
            file.close();
            
            return FromJson(json_data);
        } catch (const std::exception& e) {
            return Result::Error(ErrorCode::FileNotFound, 
                "Exception while loading config: " + std::string(e.what()));
        }
    }
    
    Result AppConfig::SaveToFile(const std::string& config_file_path) const {
        try {
            // Create directory if it doesn't exist
            std::filesystem::path file_path(config_file_path);
            std::filesystem::path dir_path = file_path.parent_path();
            if (!dir_path.empty() && !std::filesystem::exists(dir_path)) {
                std::filesystem::create_directories(dir_path);
            }
            
            std::ofstream file(config_file_path);
            if (!file.is_open()) {
                return Result::Error(ErrorCode::FileNotFound, 
                    "Cannot create config file: " + config_file_path);
            }
            
            file << ToJson();
            file.close();
            
            if (file.fail()) {
                return Result::Error(ErrorCode::FileNotFound, 
                    "Failed to write config file: " + config_file_path);
            }
            
            return Result::Success();
        } catch (const std::exception& e) {
            return Result::Error(ErrorCode::FileNotFound, 
                "Exception while saving config: " + std::string(e.what()));
        }
    }
    
    std::string AppConfig::ToJson() const {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"version\": \"" << EscapeJsonString(version) << "\",\n";
        oss << "  \"color_scheme\": " << color_scheme.ToJson() << ",\n";
        oss << "  \"key_bindings\": " << key_bindings.ToJson() << ",\n";
        oss << "  \"last_opened_file\": \"" << EscapeJsonString(last_opened_file) << "\",\n";
        oss << "  \"recent_files\": [\n";
        
        for (size_t i = 0; i < recent_files.size(); ++i) {
            oss << "    \"" << EscapeJsonString(recent_files[i]) << "\"";
            if (i < recent_files.size() - 1) oss << ",";
            oss << "\n";
        }
        
        oss << "  ],\n";
        oss << "  \"max_recent_files\": " << max_recent_files << ",\n";
        oss << "  \"auto_save_config\": " << (auto_save_config ? "true" : "false") << ",\n";
        oss << "  \"enable_real_time_monitoring\": " << (enable_real_time_monitoring ? "true" : "false") << ",\n";
        oss << "  \"file_monitor_poll_interval_ms\": " << file_monitor_poll_interval_ms << ",\n";
        oss << "  \"enable_syntax_highlighting\": " << (enable_syntax_highlighting ? "true" : "false") << ",\n";
        oss << "  \"show_line_numbers\": " << (show_line_numbers ? "true" : "false") << ",\n";
        oss << "  \"word_wrap\": " << (word_wrap ? "true" : "false") << ",\n";
        oss << "  \"max_log_entries\": " << max_log_entries << ",\n";
        oss << "  \"auto_scroll_to_bottom\": " << (auto_scroll_to_bottom ? "true" : "false") << ",\n";
        oss << "  \"default_log_level_filter\": \"" << EscapeJsonString(default_log_level_filter) << "\"\n";
        oss << "}";
        
        return oss.str();
    }  
  
    Result AppConfig::FromJson(const std::string& json_data) {
        try {
            // Simple JSON parsing - extract values
            auto extract_string = [&](const std::string& key) -> std::string {
                std::string search_key = "\"" + key + "\": \"";
                size_t start = json_data.find(search_key);
                if (start == std::string::npos) return "";
                start += search_key.length();
                size_t end = json_data.find("\"", start);
                if (end == std::string::npos) return "";
                return UnescapeJsonString(json_data.substr(start, end - start));
            };
            
            auto extract_int = [&](const std::string& key) -> int {
                std::string search_key = "\"" + key + "\": ";
                size_t start = json_data.find(search_key);
                if (start == std::string::npos) return 0;
                start += search_key.length();
                size_t end = json_data.find_first_of(",\n}", start);
                if (end == std::string::npos) return 0;
                try {
                    return std::stoi(json_data.substr(start, end - start));
                } catch (...) {
                    return 0;
                }
            };
            
            auto extract_bool = [&](const std::string& key) -> bool {
                std::string search_key = "\"" + key + "\": ";
                size_t start = json_data.find(search_key);
                if (start == std::string::npos) return false;
                start += search_key.length();
                return json_data.substr(start, 4) == "true";
            };
            
            // Extract basic values
            version = extract_string("version");
            last_opened_file = extract_string("last_opened_file");
            max_recent_files = extract_int("max_recent_files");
            auto_save_config = extract_bool("auto_save_config");
            enable_real_time_monitoring = extract_bool("enable_real_time_monitoring");
            file_monitor_poll_interval_ms = extract_int("file_monitor_poll_interval_ms");
            enable_syntax_highlighting = extract_bool("enable_syntax_highlighting");
            show_line_numbers = extract_bool("show_line_numbers");
            word_wrap = extract_bool("word_wrap");
            max_log_entries = extract_int("max_log_entries");
            auto_scroll_to_bottom = extract_bool("auto_scroll_to_bottom");
            default_log_level_filter = extract_string("default_log_level_filter");
            
            // Extract color scheme
            size_t color_start = json_data.find("\"color_scheme\": {");
            if (color_start != std::string::npos) {
                size_t brace_count = 1; // Start with 1 because we're already inside the first brace
                size_t pos = color_start + 17; // Skip to opening brace
                size_t scheme_start = pos;
                
                for (; pos < json_data.length(); ++pos) {
                    if (json_data[pos] == '{') brace_count++;
                    else if (json_data[pos] == '}') {
                        brace_count--;
                        if (brace_count == 0) break;
                    }
                }
                
                if (brace_count == 0) {
                    std::string scheme_json = json_data.substr(scheme_start, pos - scheme_start + 1);
                    auto scheme = ColorScheme::FromJson(scheme_json);
                    if (scheme) {
                        color_scheme = *scheme;
                    }
                }
            }
            
            // Extract key bindings
            size_t bindings_start = json_data.find("\"key_bindings\": {");
            if (bindings_start != std::string::npos) {
                size_t brace_count = 1; // Start with 1 because we're already inside the first brace
                size_t pos = bindings_start + 17; // Skip to opening brace
                size_t binding_start = pos;
                
                for (; pos < json_data.length(); ++pos) {
                    if (json_data[pos] == '{') brace_count++;
                    else if (json_data[pos] == '}') {
                        brace_count--;
                        if (brace_count == 0) break;
                    }
                }
                
                if (brace_count == 0) {
                    std::string bindings_json = json_data.substr(binding_start, pos - binding_start + 1);
                    auto bindings = KeyBindings::FromJson(bindings_json);
                    if (bindings) {
                        key_bindings = *bindings;
                    }
                }
            }
            
            // Extract recent files array
            recent_files.clear();
            size_t files_start = json_data.find("\"recent_files\": [");
            if (files_start != std::string::npos) {
                size_t array_start = files_start + 17;
                size_t array_end = json_data.find("]", array_start);
                if (array_end != std::string::npos) {
                    std::string files_section = json_data.substr(array_start, array_end - array_start);
                    
                    // Extract file paths from array
                    std::regex file_pattern("\"([^\"]+)\"");
                    std::sregex_iterator iter(files_section.begin(), files_section.end(), file_pattern);
                    std::sregex_iterator end;
                    
                    for (; iter != end; ++iter) {
                        recent_files.push_back(UnescapeJsonString((*iter)[1].str()));
                    }
                }
            }
            
            return Result::Success();
        } catch (const std::exception& e) {
            return Result::Error(ErrorCode::InvalidLogFormat, 
                "Exception while parsing config JSON: " + std::string(e.what()));
        }
    }
    
    AppConfig AppConfig::GetDefaultConfig() {
        AppConfig config;
        config.ApplyDefaults();
        return config;
    }
    
    Result AppConfig::MigrateFromVersion(const std::string& from_version) {
        // Simple version migration - in a real implementation, you'd have
        // specific migration logic for each version
        if (from_version.empty() || from_version == "1.0") {
            // No migration needed for current version
            return Result::Success();
        }
        
        // For now, just apply defaults for unknown versions
        ApplyDefaults();
        return Result::Success();
    }
    
    void AppConfig::ApplyDefaults() {
        version = "1.0";
        color_scheme = ColorScheme::GetDefaultDarkScheme();
        key_bindings = KeyBindings::GetDefaultKeyBindings();
        last_opened_file = "";
        recent_files.clear();
        max_recent_files = 10;
        auto_save_config = true;
        enable_real_time_monitoring = true;
        file_monitor_poll_interval_ms = 100;
        enable_syntax_highlighting = true;
        show_line_numbers = true;
        word_wrap = false;
        max_log_entries = 100000;
        auto_scroll_to_bottom = true;
        default_log_level_filter = "";
    }
    
    std::string AppConfig::EscapeJsonString(const std::string& str) const {
        std::string escaped;
        for (char c : str) {
            switch (c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }
    
    std::string AppConfig::UnescapeJsonString(const std::string& str) {
        std::string unescaped;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[i + 1]) {
                    case '"': unescaped += '"'; i++; break;
                    case '\\': unescaped += '\\'; i++; break;
                    case 'n': unescaped += '\n'; i++; break;
                    case 'r': unescaped += '\r'; i++; break;
                    case 't': unescaped += '\t'; i++; break;
                    default: unescaped += str[i]; break;
                }
            } else {
                unescaped += str[i];
            }
        }
        return unescaped;
    } 
   
    // ConfigManager implementation
    
    ConfigManager::ConfigManager() 
        : config_loaded(false) {
        current_config = AppConfig::GetDefaultConfig();
    }
    
    ConfigManager::ConfigManager(const std::string& config_path) 
        : config_file_path(config_path), config_loaded(false) {
        current_config = AppConfig::GetDefaultConfig();
    }
    
    Result ConfigManager::LoadConfig() {
        if (config_file_path.empty()) {
            config_file_path = GetDefaultConfigPath();
        }
        
        return LoadConfig(config_file_path);
    }
    
    Result ConfigManager::LoadConfig(const std::string& config_path) {
        SetConfigPath(config_path);
        
        if (!ConfigFileExists(config_path)) {
            // Create default config file
            current_config = AppConfig::GetDefaultConfig();
            Result save_result = SaveConfig(config_path);
            if (save_result.IsError()) {
                return save_result;
            }
            config_loaded = true;
            return Result::Success();
        }
        
        Result load_result = current_config.LoadFromFile(config_path);
        if (load_result.IsError()) {
            return load_result;
        }
        
        // Validate loaded config
        if (!current_config.IsValid()) {
            return Result::Error(ErrorCode::InvalidLogFormat, 
                "Loaded config is invalid: " + current_config.GetValidationError());
        }
        
        config_loaded = true;
        return Result::Success();
    }
    
    Result ConfigManager::SaveConfig() {
        if (config_file_path.empty()) {
            config_file_path = GetDefaultConfigPath();
        }
        
        return SaveConfig(config_file_path);
    }
    
    Result ConfigManager::SaveConfig(const std::string& config_path) {
        SetConfigPath(config_path);
        
        if (!current_config.IsValid()) {
            return Result::Error(ErrorCode::InvalidLogFormat, 
                "Cannot save invalid config: " + current_config.GetValidationError());
        }
        
        return current_config.SaveToFile(config_path);
    }
    
    Result ConfigManager::ResetToDefaults() {
        current_config = AppConfig::GetDefaultConfig();
        config_loaded = true;
        
        if (current_config.Get_auto_save_config() && !config_file_path.empty()) {
            return SaveConfig();
        }
        
        return Result::Success();
    }
    
    AppConfig& ConfigManager::GetConfig() {
        return current_config;
    }
    
    const AppConfig& ConfigManager::GetConfig() const {
        return current_config;
    }
    
    void ConfigManager::SetConfig(const AppConfig& config) {
        current_config = config;
        
        if (current_config.Get_auto_save_config() && !config_file_path.empty()) {
            SaveConfig(); // Ignore result for auto-save
        }
    }
    
    bool ConfigManager::IsConfigValid() const {
        return current_config.IsValid();
    }
    
    std::string ConfigManager::GetConfigValidationError() const {
        return current_config.GetValidationError();
    }
    
    std::string ConfigManager::GetDefaultConfigPath() const {
        // Get user's home directory and create config path
        std::string home_dir;
        
        #ifdef _WIN32
            const char* home = std::getenv("USERPROFILE");
            if (!home) {
                home = std::getenv("HOMEDRIVE");
                const char* homepath = std::getenv("HOMEPATH");
                if (home && homepath) {
                    home_dir = std::string(home) + std::string(homepath);
                } else {
                    home_dir = "C:\\";
                }
            } else {
                home_dir = home;
            }
            return home_dir + "\\.unreal_log_viewer\\config.json";
        #else
            const char* home = std::getenv("HOME");
            if (!home) {
                home_dir = "/tmp";
            } else {
                home_dir = home;
            }
            return home_dir + "/.config/unreal_log_viewer/config.json";
        #endif
    }
    
    bool ConfigManager::ConfigFileExists() const {
        return ConfigFileExists(config_file_path);
    }
    
    bool ConfigManager::ConfigFileExists(const std::string& config_path) const {
        return std::filesystem::exists(config_path) && 
               std::filesystem::is_regular_file(config_path);
    }
    
    void ConfigManager::SetConfigPath(const std::string& config_path) {
        config_file_path = config_path;
    }
    
} // namespace ue_log