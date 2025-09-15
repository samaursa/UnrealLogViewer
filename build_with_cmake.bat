# Build the UnrealLogViewer with CMake
cd /d "D:\Repos\UnrealLogViewer"

# Clean and configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build build --config Release --parallel

echo "✅ Build completed! Updated executable is at: build\Release\unreal_log_viewer.exe"
