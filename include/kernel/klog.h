#ifndef KLOG_H
#define KLOG_H

enum klog_level {
    LOG_INFO,
    LOG_WARN,
    LOG_ERR
};

void klog(char level, const char *namespace, const char* s);

#endif