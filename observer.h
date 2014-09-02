#ifndef __OBSERVER_H_JUN__
#define __OBSERVER_H_JUN__

#include <memory>

#define TERMINATE_THREAD (-1)
#define THE_OTHER_IS_DEAD  (-2)
#define THE_OTHER_IS_ALIVE (-3)

class observer
{
public:
	observer() {}
	virtual ~observer() {};

	virtual void update(int flag) = 0;
};

class subject
{
public:
	subject() {}
	virtual ~subject() {}
	
	virtual void attach_observer(std::shared_ptr<observer> obs) = 0;
	virtual void notify_observers(int flag) = 0;
};

#endif
