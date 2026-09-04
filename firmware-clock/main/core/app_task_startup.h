// 声明常驻应用任务的统一创建入口。
#pragma once

// Start the physical input service before the network/boot-animation wait so
// BOOT + KEY can always recover panel ownership during startup.
bool start_button_task_early();
void create_regular_app_tasks();
