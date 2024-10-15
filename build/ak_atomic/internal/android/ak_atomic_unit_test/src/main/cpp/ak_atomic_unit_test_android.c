#include <ak_atomic_unit_test.c>
#include <android/log.h>
#include <android_native_app_glue.h>

UTEST_STATE();

static int pfd[2];
static pthread_t thr;

typedef struct {
    bool IsReady;
    bool IsFinished;
} ak_android_test_context;

static bool Android_Poll_App(struct android_app* App) {
    int EventID;
    int Events;

    struct android_poll_source* Source;

    while((EventID = ALooper_pollOnce(0, NULL, &Events, (void**)&Source)) >= 0) {
        if(Source != NULL) {
            Source->process(App, Source);
        }

        if(App->destroyRequested != 0) {
            //End app
            return false;
        }
    }

    return true;
}

static int32_t Android_Event_Proc(struct android_app* App, AInputEvent* Event) {
    return 0;
}

static void Android_On_App_Cmd(struct android_app* App, int32_t Cmd) {
    ak_android_test_context* Context = (ak_android_test_context*)App->userData;

    switch(Cmd) {
        case APP_CMD_INIT_WINDOW: {
            if(App->window != NULL) {
                Context->IsReady = true;
            }
        } break;
    }
}

static void* Android_Thread_Func(void* Parameter)
{
    ssize_t rdsz;
    char buf[1024];
    while((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0) {
        if(buf[rdsz - 1] == '\n') --rdsz;
        buf[rdsz] = 0;  /* add null-terminator */
        __android_log_write(ANDROID_LOG_DEBUG, "Custom", buf);
    }
    return 0;
}

void android_main(struct android_app* App) {
	//Need to route stdout requests to the android logger
	// make stdout line-buffered and stderr unbuffered */
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    // create the pipe and redirect stdout and stderr */
    pipe(pfd);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    /* spawn the logging thread */
    if(pthread_create(&thr, 0, Android_Thread_Func, 0) == -1)
        return;
    pthread_detach(thr);

	ak_android_test_context TestContext = { 0 };

	App->userData = &TestContext;
	App->onAppCmd = Android_On_App_Cmd;
	App->onInputEvent = Android_Event_Proc;

	for (;;) {
		if (!Android_Poll_App(App)) {
			return;   
		}

		if (TestContext.IsReady && !TestContext.IsFinished) {
			const char* Filters[] = {
                "",
                "--filter="
            };
			
			utest_main(0, Filters);
			ANativeActivity_finish(App->activity);
			TestContext.IsFinished = true;
		}
	}
}