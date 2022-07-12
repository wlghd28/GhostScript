#include "callbacks.h"

#include "jni_util.h"

#include <string.h>

#define STDIN_SIG "(J[BI)I"
#define STDOUT_SIG "(J[BI)I"
#define STDERR_SIG "(J[BI)I"

#define POLL_SIG "(J)I"

#define DISPLAY_OPEN_SIG "(JJ)I"
#define DISPLAY_PRECLOSE_SIG "(JJ)I"
#define DISPLAY_CLOSE_SIG "(JJ)I"
#define DISPLAY_PRESIZE_SIG "(JJIIII)I"
#define DISPLAY_SIZE_SIG "(JJIIIILcom/artifex/gsjava/util/BytePointer;)I"
#define DISPLAY_SYNC_SIG "(JJ)I"
#define DISPLAY_PAGE_SIG "(JJIZ)I"
#define DISPLAY_UPDATE_SIG "(JJIIII)I"
// display memalloc
// display memfree
#define DISPLAY_SEPARATION_SIG "(JJI[BSSSS)I"
#define DISPLAY_ADJUST_BAND_HEIGHT_SIG "(JJI)I"
#define DISPLAY_RECTANGLE_REQUEST "(JJLcom/artifex/gsjava/LongReference;Lcom/artifex/gsjava/IntReference;\
Lcom/artifex/gsjava/IntReference;Lcom/artifex/gsjava/IntReference;Lcom/artifex/gsjava/IntReference;\
Lcom/artifex/gsjava/IntReference;Lcom/artifex/gsjava/IntReference;Lcom/artifex/gsjava/IntReference;\
Lcom/artifex/gsjava/IntReference;)I"

#define CHECK_AND_RETURN(E) if (E->ExceptionCheck()) { return -21; }

using namespace util;

static JNIEnv *g_env = NULL;

static jobject g_stdIn = NULL;
static jobject g_stdOut = NULL;
static jobject g_stdErr = NULL;

static jobject g_poll = NULL;

static jobject g_displayCallback = NULL;

static jobject g_callout = NULL;

void callbacks::setJNIEnv(JNIEnv *env)
{
	g_env = env;
}

void callbacks::setIOCallbacks(jobject stdIn, jobject stdOut, jobject stdErr)
{
	if (g_env)
	{
		if (g_stdIn)
			g_env->DeleteGlobalRef(g_stdIn);

		if (g_stdOut)
			g_env->DeleteGlobalRef(g_stdOut);

		if (g_stdErr)
			g_env->DeleteGlobalRef(g_stdErr);

		g_stdIn = g_env->NewGlobalRef(stdIn);
		g_stdOut = g_env->NewGlobalRef(stdOut);
		g_stdErr = g_env->NewGlobalRef(stdErr);
	}
}

int callbacks::stdInFunction(void *callerHandle, char *buf, int len)
{
	int code = 0;
	if (g_env && g_stdIn)
	{
		jbyteArray byteArray = g_env->NewByteArray(len);
		g_env->SetByteArrayRegion(byteArray, 0, len, (jbyte *)buf);
		code = callIntMethod(g_env, g_stdIn, "onStdIn", STDIN_SIG, (jlong)callerHandle, byteArray, (jint)len);
	}
	return code;
}

int callbacks::stdOutFunction(void *callerHandle, const char *str, int len)
{
	int code = 0;
	if (g_env && g_stdOut)
	{
		jbyteArray byteArray = g_env->NewByteArray(len);
		g_env->SetByteArrayRegion(byteArray, 0, len, (const jbyte *)str);
		code = callIntMethod(g_env, g_stdOut, "onStdOut", STDOUT_SIG, (jlong)callerHandle, byteArray, (jint)len);
	}
	return code;
}

int callbacks::stdErrFunction(void *callerHandle, const char *str, int len)
{
	int code = 0;
	if (g_env && g_stdErr)
	{
		jbyteArray byteArray = g_env->NewByteArray(len);
		g_env->SetByteArrayRegion(byteArray, 0, len, (const jbyte *)str);
		code = callIntMethod(g_env, g_stdErr, "onStdErr", STDERR_SIG, (jlong)callerHandle, byteArray, (jint)len);
	}
	return code;
}

void callbacks::setPollCallback(jobject poll)
{
	if (g_env)
	{
		if (g_poll)
			g_env->DeleteGlobalRef(g_poll);

		g_poll = g_env->NewGlobalRef(poll);
	}
}

int callbacks::pollFunction(void *callerHandle)
{
	int code = 0;
	if (g_env && g_poll)
	{
		code = callIntMethod(g_env, g_poll, "onPoll", POLL_SIG, (jlong)callerHandle);
	}
	return code;
}

void callbacks::setDisplayCallback(jobject displayCallback)
{
	if (g_env)
	{
		if (g_displayCallback)
		{
			g_env->DeleteGlobalRef(g_displayCallback);
			g_displayCallback = NULL;
		}

		g_displayCallback = g_env->NewGlobalRef(displayCallback);
		//g_displayCallback = displayCallback;
	}
}

void callbacks::setCalloutCallback(jobject callout)
{
	if (g_env)
	{
		if (g_callout)
			g_env->DeleteGlobalRef(g_callout);

		g_callout = g_env->NewGlobalRef(callout);
	}
}

int callbacks::calloutFunction(void *instance, void *handle, const char *deviceName, int id, int size, void *data)
{
	int code = 0;
	if (g_env && g_callout)
	{
		jsize len = strlen(deviceName);
		jbyteArray array = g_env->NewByteArray(len);
		g_env->SetByteArrayRegion(array, 0, len, (const jbyte *)deviceName);
		code = callIntMethod(g_env, g_callout, "onCallout", "(JJ[BIIJ)I", (jlong)instance, (jlong)handle, array, id, size, (jlong)data);
	}
	return code;
}

int callbacks::display::displayOpenFunction(void *handle, void *device)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		jclass clazz = g_env->GetObjectClass(g_displayCallback);
		const char *name = getClassName(g_env, clazz);
		freeClassName(name);
		code = callIntMethod(g_env, g_displayCallback, "onDisplayOpen", DISPLAY_OPEN_SIG, (jlong)handle, (jlong)device);
		CHECK_AND_RETURN(g_env);
	}
	return 0;
}

int callbacks::display::displayPrecloseFunction(void *handle, void *device)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		code = callIntMethod(g_env, g_displayCallback, "onDisplayPreclose", DISPLAY_PRECLOSE_SIG, (jlong)handle, (jlong)device);
		CHECK_AND_RETURN(g_env);
	}
	return code;
}

int callbacks::display::displayCloseFunction(void *handle, void *device)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		code = callIntMethod(g_env, g_displayCallback, "onDisplayClose", DISPLAY_CLOSE_SIG, (jlong)handle, (jlong)device);
		CHECK_AND_RETURN(g_env);
	}
	return code;
}

int callbacks::display::displayPresizeFunction(void *handle, void *device, int width, int height, int raster, unsigned int format)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		code = callIntMethod(g_env, g_displayCallback, "onDisplayPresize", DISPLAY_PRESIZE_SIG, (jlong)handle,
			(jlong)device, width, height, raster, (jint)format);
		CHECK_AND_RETURN(g_env);
	}
	return code;
}

int callbacks::display::displaySizeFunction(void *handle, void *device, int width, int height, int raster,
	unsigned int format, unsigned char *pimage)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		jsize len = height * raster;
		jbyteArray byteArray = g_env->NewByteArray(len);
		g_env->SetByteArrayRegion(byteArray, 0, len, (signed char *)pimage);

		static const char *const bytePointerClassName = "com/artifex/gsjava/util/BytePointer";
		static const char *const nativePointerClassName = "com/artifex/gsjava/util/NativePointer";

		jclass bytePointerClass = g_env->FindClass(bytePointerClassName);
		if (bytePointerClass == NULL)
		{
			throwNoClassDefError(g_env, bytePointerClassName);
			return -21;
		}

		jclass nativePointerClass = g_env->FindClass(nativePointerClassName);
		if (nativePointerClass == NULL)
		{
			throwNoClassDefError(g_env, nativePointerClassName);
			return -21;
		}

		jmethodID constructor = g_env->GetMethodID(bytePointerClass, "<init>", "()V");
		if (constructor == NULL)
		{
			throwNoSuchMethodError(g_env, "com.artifex.gsjava.util.BytePointer.<init>()V");
			return -21;
		}
		jobject bytePointer = g_env->NewObject(bytePointerClass, constructor);

		jfieldID dataPtrID = g_env->GetFieldID(nativePointerClass, "address", "J");
		if (dataPtrID == NULL)
		{
			throwNoSuchFieldError(g_env, "address");
			return -21;
		}

		jfieldID lengthID = g_env->GetFieldID(bytePointerClass, "length", "J");
		if (lengthID == NULL)
		{
			throwNoSuchFieldError(g_env, "length");
			return -21;
		}

		g_env->SetLongField(bytePointer, dataPtrID, (jlong)pimage);
		g_env->SetLongField(bytePointer, lengthID, len);

		code = callIntMethod(g_env, g_displayCallback, "onDisplaySize", DISPLAY_SIZE_SIG, (jlong)handle,
			(jlong)device, width, height, raster, (jint)format, bytePointer);
		CHECK_AND_RETURN(g_env);
	}
	return code;
}

int callbacks::display::displaySyncFunction(void *handle, void *device)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		code = callIntMethod(g_env, g_displayCallback, "onDisplaySync", DISPLAY_SYNC_SIG, (jlong)handle, (jlong)device);
		CHECK_AND_RETURN(g_env);
	}
	return code;
}

int callbacks::display::displayPageFunction(void *handle, void *device, int copies, int flush)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		code = callIntMethod(g_env, g_displayCallback, "onDisplayPage", DISPLAY_PAGE_SIG, (jlong)handle,
			(jlong)device, copies, flush);
		CHECK_AND_RETURN(g_env);
	}
	return code;
}

int callbacks::display::displayUpdateFunction(void *handle, void *device, int x, int y, int w, int h)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		code = callIntMethod(g_env, g_displayCallback, "onDisplayUpdate", DISPLAY_UPDATE_SIG, (jlong)handle,
			(jlong)device, x, y, w, h);
		CHECK_AND_RETURN(g_env);
	}
	return code;
}

int callbacks::display::displaySeparationFunction(void *handle, void *device, int component, const char *componentName,
	unsigned short c, unsigned short m, unsigned short y, unsigned short k)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		jsize len = strlen(componentName);
		jbyteArray byteArray = g_env->NewByteArray(len);
		g_env->SetByteArrayRegion(byteArray, 0, len, (const jbyte *)componentName);
		code = callIntMethod(g_env, g_displayCallback, "onDisplaySeparation", DISPLAY_SEPARATION_SIG, (jlong)handle,
			(jlong)device, component, byteArray, c, m, y, k);
		CHECK_AND_RETURN(g_env);
	}
	return code;
}

int callbacks::display::displayAdjustBandHeightFunction(void *handle, void *device, int bandHeight)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		code = callIntMethod(g_env, g_displayCallback, "onDisplayAdjustBandHeght", DISPLAY_ADJUST_BAND_HEIGHT_SIG,
			(jlong)handle, (jlong)device, bandHeight);
		CHECK_AND_RETURN(g_env);
	}
	return code;
}

int callbacks::display::displayRectangleRequestFunction(void *handle, void *device, void **memory, int *ox, int *oy,
	int *raster, int *plane_raster, int *x, int *y, int *w, int *h)
{
	int code = 0;
	if (g_env && g_displayCallback)
	{
		Reference memoryRef = Reference(g_env, toWrapperType(g_env, (jlong)*memory));
		Reference oxRef = Reference(g_env, toWrapperType(g_env, (jint)*ox));
		Reference oyRef = Reference(g_env, toWrapperType(g_env, (jint)*oy));
		Reference rasterRef = Reference(g_env, toWrapperType(g_env, (jint)*raster));
		Reference planeRasterRef = Reference(g_env, toWrapperType(g_env, (jint)*plane_raster));
		Reference xRef = Reference(g_env, toWrapperType(g_env, (jint)*x));
		Reference yRef = Reference(g_env, toWrapperType(g_env, (jint)*y));
		Reference wRef = Reference(g_env, toWrapperType(g_env, (jint)*w));
		Reference hRef = Reference(g_env, toWrapperType(g_env, (jint)*h));

		code = callIntMethod(g_env, g_displayCallback, "onDisplayRectangleRequest", DISPLAY_RECTANGLE_REQUEST,
			(jlong)handle, (jlong)device, memoryRef, oxRef, oyRef, rasterRef, planeRasterRef, xRef, yRef, wRef, hRef);

		*memory = (void *)memoryRef.longValue();
		*ox = oxRef.intValue();
		*oy = oyRef.intValue();
		*raster = rasterRef.intValue();
		*plane_raster = planeRasterRef.intValue();
		*x = xRef.intValue();
		*y = yRef.intValue();
		*w = wRef.intValue();
		*h = hRef.intValue();

		CHECK_AND_RETURN(g_env);
	}
	return code;
}
