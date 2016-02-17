
#ifdef TIM_DEBUG
#define new new(__LINE__,__FILE__)
#define delete tim::core::MemoryLogger::instance().nextDealloc(__LINE__,__FILE__),delete
#endif

