#include "lunaris/core/application.h"

int main() {
    lunaris::Application app;
    
    tvk::AppConfig config;
    config.title = "Lunaris";
    config.width = 1600;
    config.height = 900;
    config.vsync = true;
    config.mode = tvk::AppMode::GUI;
    config.enableDockspace = true;
    
    app.Run(config);
    
    return 0;
}
