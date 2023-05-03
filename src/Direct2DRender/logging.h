#define SafeRelease(p) { if (p) { (p)->Release(); (p) = nullptr; } }

#define LOG_I(message) printf("[INFO] %s\n", message)
#define LOG_W(message) printf("[Warning] %s\n", message)
#define LOG_E(message) printf("[Error] %s\n", message)