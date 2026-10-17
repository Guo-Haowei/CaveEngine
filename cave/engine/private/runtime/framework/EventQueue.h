#pragma once
#include "cave/core/ids/ViewId.h"

namespace cave {

class EventListener;
class Scene;

class IEvent {
public:
    IEvent() {}

    virtual ~IEvent() = default;
};

class ResizeEvent : public IEvent {
public:
    ResizeEvent(int width, int height)
        : m_width(width), m_height(height) {}

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

protected:
    int m_width;
    int m_height;
};

struct ViewCameraChangedEvent : public IEvent {
    ViewId view_id;
};

class EventListener {
public:
    virtual ~EventListener() {}
    virtual void eventReceived(const IEvent* event) = 0;
};

class EventQueue {
public:
    ~EventQueue();

    void enqueueEvent(Owner<IEvent> event);

    void dispatchEvent(const IEvent* event);

    void flushEvents();

    void registerListener(EventListener* listener);

    void unregisterListener(EventListener* listener);

private:
    std::queue<Owner<IEvent>> m_events;
    Vector<EventListener*> m_listeners;
};

}  // namespace cave
