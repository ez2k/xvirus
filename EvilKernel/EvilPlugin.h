#if !defined(__EVILPLUGIN_H__)
#define __EVILPLUGIN_H__

#include "Common.h"


#if defined(__cplusplus)
extern "C"
{
#endif

// �����ʼ��
typedef __bool (__API__ *FPEvilPluginInit)();
// �������
typedef __bool (__API__ *FPEvilPluginRun)();

#if defined(__cplusplus)
}
#endif

#endif
