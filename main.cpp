#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Needs a class name argument\n");
        return 1;
    }

    Display* dpy = XOpenDisplay(getenv("DISPLAY"));
    if (!dpy) {
        fprintf(stderr, "Couldn't open display\n");
        return 2;
    }

    const Atom clientListAtom = XInternAtom(dpy, "_NET_CLIENT_LIST", True);
    const Atom activeWindowAtom = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);

    Atom actualType;
    int format;
    unsigned long numItems, bytesAfter;
    unsigned char *data = 0;
    XClassHint cls;

    Window root;
    Window target;
    bool found = false;
    const int screens = XScreenCount(dpy);
    for (int i = 0; i < screens; ++i) {
        const int status = XGetWindowProperty(dpy, XRootWindow(dpy, i), clientListAtom, 0L, (~0L), False,
                                              AnyPropertyType, &actualType, &format, &numItems, &bytesAfter, &data);
        if (status >= Success && numItems) {
            assert(format == 32);
            assert(sizeof(int) == 4);
            unsigned int* array = reinterpret_cast<unsigned int*>(data);
            for (int j = 0; j < numItems; ++j) {
                const Window w = static_cast<Window>(array[j]);

                if (!XGetClassHint(dpy, w, &cls))
                    continue;

                if (cls.res_class) {
                    if (!strcmp(cls.res_class, argv[1])) {
                        found = true;
                        target = w;
                        root = XRootWindow(dpy, i);
                    }
                    XFree(cls.res_class);
                }
                if (cls.res_name)
                    XFree(cls.res_name);
                if (found)
                    break;
            }
            if (data)
                XFree(data);
        }
        if (found)
            break;
    }

    if (found) {
        XEvent event;
        const long mask = SubstructureRedirectMask | SubstructureNotifyMask;
        event.xclient.type = ClientMessage;
        event.xclient.serial = 0;
        event.xclient.send_event = True;
        event.xclient.display = dpy;
        event.xclient.window = target;
        event.xclient.message_type = activeWindowAtom;
        event.xclient.format = 32;
        event.xclient.data.l[0] = 1; // 1 = application, 2 = pager
        event.xclient.data.l[1] = CurrentTime;
        event.xclient.data.l[2] = 0; // senders current active window
        event.xclient.data.l[3] = 0;
        event.xclient.data.l[4] = 0;

        XSendEvent(dpy, root, False, mask, &event);
    }

    XCloseDisplay(dpy);
    return 0;
}
