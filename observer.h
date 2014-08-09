#ifndef __OBSERVER_H_JUN__
#define __OBSERVER_H_JUN__

#include <memory>

#define TERMINATE_THREAD (-1)

class observer
{
public:
	observer() {}
	virtual ~observer() = 0;

	virtual void update(int flag) = 0;
};

class subject
{
public:
	subject() {}
	~subject() {}
	
	virtual void attach_observer(std::shared_ptr<observer> obs) = 0;
	virtual void notify_observers(int flag) = 0;
};

#endif