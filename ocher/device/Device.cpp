#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#ifdef OCHER_TARGET_KOBO
#include <linux/input.h>
#endif

#include "clc/support/Logger.h"

#include "ocher/device/Version.h"
#include "ocher/device/Device.h"


Device *device;

void initDevice()
{
    device = new Device();
}

clc::Buffer Device::getMac()
{
    clc::Buffer mac;
    // TODO
    return mac;
}

clc::Buffer Device::getIp()
{
    clc::Buffer ip;
    // TODO
    return ip;
}

clc::Buffer Device::getVersion()
{
    clc::Buffer version;
    version.format("%d.%d.%d", OCHER_MAJOR, OCHER_MINOR, OCHER_PATCH);
    return version;
}

clc::Buffer Device::getBuildDate()
{
    clc::Buffer date(__DATE__);
    return date;
}


#ifdef OCHER_TARGET_KOBO
KoboDeviceThread::KoboDeviceThread()
{
    m_buttonFd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    m_touchFd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
    pipe(m_pipe);
    start();
}

KoboDeviceThread::~KoboDeviceThread()
{
    interrupt();
    write(m_pipe[1], "", 1);
    join();
    close(m_pipe[0]);
    close(m_pipe[1]);

    if (m_buttonFd != -1) {
        close(m_buttonFd);
    }
    if (m_touchFd != -1) {
        close(m_touchFd);
    }
}

struct KoboButtonEvent {
    uint16_t timeHigh;
    uint16_t res1;
    uint16_t timeLow;
    uint16_t res2;
    uint16_t res3;
    uint16_t button;
    uint16_t press;
    uint16_t res4;
};

void KoboDeviceThread::run()
{
    clc::Log::debug("ocher.kobo", "thread alive");

    int nfds = 0;
    fd_set rdfds;
    FD_ZERO(&rdfds);
    FD_SET(m_buttonFd, &rdfds);
    if (nfds < m_buttonFd)
        nfds = m_buttonFd;
    FD_SET(m_touchFd, &rdfds);
    if (nfds < m_touchFd)
        nfds = m_touchFd;
    FD_SET(m_pipe[0], &rdfds);
    if (nfds < m_pipe[0])
        nfds = m_pipe[0];

    while (1) {
        int r = select(nfds + 1, &rdfds, 0, 0, 0);
        if (r == -1) {
            clc::Log::error("ocher.kobo", "select: %s", strerror(errno));
        } else if (r == 0) {
            clc::Log::error("ocher.kobo", "select: timeout");
        } else {
            clc::Log::debug("ocher.kobo", "select: awake");

            while (1) {
                struct KoboButtonEvent evt;
                r = read(m_buttonFd, &evt, sizeof(evt));
                if (r == -1) {
                    if (errno == EINTR)
                        continue;
                    ASSERT(errno == EAGAIN || errno == EWOULDBLOCK);
                    break;
                } else if (r == sizeof(evt)) {
                    if (evt.button == 0x66) {
                        printf("HOME %s\n", evt.press ? "down" : "up");
                    } else if (evt.button == 0x74) {
                        printf("SLEEP %s\n", evt.press ? "down" : "up");
                    }
                } else {
                    break;
                }
            }

            while (1) {
                struct input_event evt[64];
                r = read(m_buttonFd, &evt, sizeof(struct input_event)*64);
                if (r == -1) {
                    if (errno == EINTR)
                        continue;
                    ASSERT(errno == EAGAIN || errno == EWOULDBLOCK);
                    break;
                } else if (r >= sizeof(struct input_event)) {
                    for (int i = 0; i < r / sizeof(struct input_event); i++) {
                        if (evt[i].type == EV_SYN) {
                            printf("Event: time %ld.%06ld, -------------- %s ------------\n",
                                    evt[i].time.tv_sec, evt[i].time.tv_usec, evt[i].code ? "Config Sync" : "Report Sync" );
                        }
                    }
                } else {
                    break;
                }
            }
        }
    }
    clc::Log::debug("ocher.kobo", "thread dead");
}

#endif
