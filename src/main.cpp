#include "lunaris/core/application.h"

int main() {
    lunaris::Application app;
    
    tvk::AppConfig config;
    config.title = "Lunaris";
    config.width = 1280;
    config.height = 720;
    config.vsync = true;
    config.mode = tvk::AppMode::GUI;
    config.enableDockspace = true;
    
    app.Run(config);
    
    return 0;
}
