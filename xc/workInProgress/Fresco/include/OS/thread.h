/* DO NOT EDIT -- Automatically generated from Interfaces/thread.idl */

#ifndef Interfaces_thread_h
#define Interfaces_thread_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/Ox/object.h>

class BaseThreadsObjType;
typedef BaseThreadsObjType* BaseThreadsObjRef;
typedef BaseThreadsObjRef BaseThreadsObj_in;
class BaseThreadsObj;
class BaseThreadsObj_tmp;
class BaseThreadsObj_var;

class BaseThreadsObj {
public:
    BaseThreadsObjRef _obj_;

    BaseThreadsObj() { _obj_ = 0; }
    BaseThreadsObj(BaseThreadsObjRef p) { _obj_ = p; }
    BaseThreadsObj& operator =(BaseThreadsObjRef p);
    BaseThreadsObj(const BaseThreadsObj&);
    BaseThreadsObj& operator =(const BaseThreadsObj& r);
    BaseThreadsObj(const BaseThreadsObj_tmp&);
    BaseThreadsObj& operator =(const BaseThreadsObj_tmp&);
    BaseThreadsObj(const BaseThreadsObj_var&);
    BaseThreadsObj& operator =(const BaseThreadsObj_var&);
    ~BaseThreadsObj();

    BaseThreadsObjRef operator ->() { return _obj_; }

    operator BaseThreadsObj_in() const { return _obj_; }
    static BaseThreadsObjRef _narrow(BaseObjectRef p);
    static BaseThreadsObj_tmp _narrow(const BaseObject& r);

    static BaseThreadsObjRef _duplicate(BaseThreadsObjRef obj);
    static BaseThreadsObj_tmp _duplicate(const BaseThreadsObj& r);
};

class BaseThreadsObj_tmp : public BaseThreadsObj {
public:
    BaseThreadsObj_tmp(BaseThreadsObjRef p) { _obj_ = p; }
    BaseThreadsObj_tmp(const BaseThreadsObj& r);
    BaseThreadsObj_tmp(const BaseThreadsObj_tmp& r);
    ~BaseThreadsObj_tmp();
};

class BaseThreadsObj_var {
public:
    BaseThreadsObjRef _obj_;

    BaseThreadsObj_var(BaseThreadsObjRef p) { _obj_ = p; }
    operator BaseThreadsObjRef() const { return _obj_; }
    BaseThreadsObjRef operator ->() { return _obj_; }
};

class BaseThreadsObjType : public BaseObjectType {
protected:
    BaseThreadsObjType();
    virtual ~BaseThreadsObjType();
public:
    virtual Long ref__(Long references) = 0;
    BaseThreadsObjRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class ThreadObjType;
typedef ThreadObjType* ThreadObjRef;
typedef ThreadObjRef ThreadObj_in;
class ThreadObj;
class ThreadObj_tmp;
class ThreadObj_var;

class ThreadObj {
public:
    ThreadObjRef _obj_;

    ThreadObj() { _obj_ = 0; }
    ThreadObj(ThreadObjRef p) { _obj_ = p; }
    ThreadObj& operator =(ThreadObjRef p);
    ThreadObj(const ThreadObj&);
    ThreadObj& operator =(const ThreadObj& r);
    ThreadObj(const ThreadObj_tmp&);
    ThreadObj& operator =(const ThreadObj_tmp&);
    ThreadObj(const ThreadObj_var&);
    ThreadObj& operator =(const ThreadObj_var&);
    ~ThreadObj();

    ThreadObjRef operator ->() { return _obj_; }

    operator ThreadObj_in() const { return _obj_; }
    operator BaseThreadsObj() const;
    static ThreadObjRef _narrow(BaseObjectRef p);
    static ThreadObj_tmp _narrow(const BaseObject& r);

    static ThreadObjRef _duplicate(ThreadObjRef obj);
    static ThreadObj_tmp _duplicate(const ThreadObj& r);
};

class ThreadObj_tmp : public ThreadObj {
public:
    ThreadObj_tmp(ThreadObjRef p) { _obj_ = p; }
    ThreadObj_tmp(const ThreadObj& r);
    ThreadObj_tmp(const ThreadObj_tmp& r);
    ~ThreadObj_tmp();
};

class ThreadObj_var {
public:
    ThreadObjRef _obj_;

    ThreadObj_var(ThreadObjRef p) { _obj_ = p; }
    operator ThreadObjRef() const { return _obj_; }
    ThreadObjRef operator ->() { return _obj_; }
};

class ThreadObjType : public BaseThreadsObjType {
protected:
    ThreadObjType();
    virtual ~ThreadObjType();
public:
    virtual void run() = 0;
    virtual void terminate() = 0;
    virtual void wait(Long& status, Long& exitcode) = 0;
    ThreadObjRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class LockObjType;
typedef LockObjType* LockObjRef;
typedef LockObjRef LockObj_in;
class LockObj;
class LockObj_tmp;
class LockObj_var;

class LockObj {
public:
    LockObjRef _obj_;

    LockObj() { _obj_ = 0; }
    LockObj(LockObjRef p) { _obj_ = p; }
    LockObj& operator =(LockObjRef p);
    LockObj(const LockObj&);
    LockObj& operator =(const LockObj& r);
    LockObj(const LockObj_tmp&);
    LockObj& operator =(const LockObj_tmp&);
    LockObj(const LockObj_var&);
    LockObj& operator =(const LockObj_var&);
    ~LockObj();

    LockObjRef operator ->() { return _obj_; }

    operator LockObj_in() const { return _obj_; }
    operator BaseThreadsObj() const;
    static LockObjRef _narrow(BaseObjectRef p);
    static LockObj_tmp _narrow(const BaseObject& r);

    static LockObjRef _duplicate(LockObjRef obj);
    static LockObj_tmp _duplicate(const LockObj& r);
};

class LockObj_tmp : public LockObj {
public:
    LockObj_tmp(LockObjRef p) { _obj_ = p; }
    LockObj_tmp(const LockObj& r);
    LockObj_tmp(const LockObj_tmp& r);
    ~LockObj_tmp();
};

class LockObj_var {
public:
    LockObjRef _obj_;

    LockObj_var(LockObjRef p) { _obj_ = p; }
    operator LockObjRef() const { return _obj_; }
    LockObjRef operator ->() { return _obj_; }
};

class LockObjType : public BaseThreadsObjType {
protected:
    LockObjType();
    virtual ~LockObjType();
public:
    virtual void acquire() = 0;
    virtual void release() = 0;
    virtual Boolean try_acquire() = 0;
    LockObjRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class ConditionVariableType;
typedef ConditionVariableType* ConditionVariableRef;
typedef ConditionVariableRef ConditionVariable_in;
class ConditionVariable;
class ConditionVariable_tmp;
class ConditionVariable_var;

class ConditionVariable {
public:
    ConditionVariableRef _obj_;

    ConditionVariable() { _obj_ = 0; }
    ConditionVariable(ConditionVariableRef p) { _obj_ = p; }
    ConditionVariable& operator =(ConditionVariableRef p);
    ConditionVariable(const ConditionVariable&);
    ConditionVariable& operator =(const ConditionVariable& r);
    ConditionVariable(const ConditionVariable_tmp&);
    ConditionVariable& operator =(const ConditionVariable_tmp&);
    ConditionVariable(const ConditionVariable_var&);
    ConditionVariable& operator =(const ConditionVariable_var&);
    ~ConditionVariable();

    ConditionVariableRef operator ->() { return _obj_; }

    operator ConditionVariable_in() const { return _obj_; }
    operator BaseThreadsObj() const;
    static ConditionVariableRef _narrow(BaseObjectRef p);
    static ConditionVariable_tmp _narrow(const BaseObject& r);

    static ConditionVariableRef _duplicate(ConditionVariableRef obj);
    static ConditionVariable_tmp _duplicate(const ConditionVariable& r);
};

class ConditionVariable_tmp : public ConditionVariable {
public:
    ConditionVariable_tmp(ConditionVariableRef p) { _obj_ = p; }
    ConditionVariable_tmp(const ConditionVariable& r);
    ConditionVariable_tmp(const ConditionVariable_tmp& r);
    ~ConditionVariable_tmp();
};

class ConditionVariable_var {
public:
    ConditionVariableRef _obj_;

    ConditionVariable_var(ConditionVariableRef p) { _obj_ = p; }
    operator ConditionVariableRef() const { return _obj_; }
    ConditionVariableRef operator ->() { return _obj_; }
};

class ConditionVariableType : public BaseThreadsObjType {
protected:
    ConditionVariableType();
    virtual ~ConditionVariableType();
public:
    virtual void wait(LockObj_in lock) = 0;
    virtual void notify() = 0;
    virtual void broadcast() = 0;
    ConditionVariableRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class SemaphoreType;
typedef SemaphoreType* SemaphoreRef;
typedef SemaphoreRef Semaphore_in;
class Semaphore;
class Semaphore_tmp;
class Semaphore_var;

class Semaphore {
public:
    SemaphoreRef _obj_;

    Semaphore() { _obj_ = 0; }
    Semaphore(SemaphoreRef p) { _obj_ = p; }
    Semaphore& operator =(SemaphoreRef p);
    Semaphore(const Semaphore&);
    Semaphore& operator =(const Semaphore& r);
    Semaphore(const Semaphore_tmp&);
    Semaphore& operator =(const Semaphore_tmp&);
    Semaphore(const Semaphore_var&);
    Semaphore& operator =(const Semaphore_var&);
    ~Semaphore();

    SemaphoreRef operator ->() { return _obj_; }

    operator Semaphore_in() const { return _obj_; }
    operator BaseThreadsObj() const;
    static SemaphoreRef _narrow(BaseObjectRef p);
    static Semaphore_tmp _narrow(const BaseObject& r);

    static SemaphoreRef _duplicate(SemaphoreRef obj);
    static Semaphore_tmp _duplicate(const Semaphore& r);
};

class Semaphore_tmp : public Semaphore {
public:
    Semaphore_tmp(SemaphoreRef p) { _obj_ = p; }
    Semaphore_tmp(const Semaphore& r);
    Semaphore_tmp(const Semaphore_tmp& r);
    ~Semaphore_tmp();
};

class Semaphore_var {
public:
    SemaphoreRef _obj_;

    Semaphore_var(SemaphoreRef p) { _obj_ = p; }
    operator SemaphoreRef() const { return _obj_; }
    SemaphoreRef operator ->() { return _obj_; }
};

class SemaphoreType : public BaseThreadsObjType {
protected:
    SemaphoreType();
    virtual ~SemaphoreType();
public:
    virtual void P() = 0;
    virtual void V() = 0;
    SemaphoreRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class ThreadKitType;
typedef ThreadKitType* ThreadKitRef;
typedef ThreadKitRef ThreadKit_in;
class ThreadKit;
class ThreadKit_tmp;
class ThreadKit_var;

class ThreadKit {
public:
    ThreadKitRef _obj_;

    ThreadKit() { _obj_ = 0; }
    ThreadKit(ThreadKitRef p) { _obj_ = p; }
    ThreadKit& operator =(ThreadKitRef p);
    ThreadKit(const ThreadKit&);
    ThreadKit& operator =(const ThreadKit& r);
    ThreadKit(const ThreadKit_tmp&);
    ThreadKit& operator =(const ThreadKit_tmp&);
    ThreadKit(const ThreadKit_var&);
    ThreadKit& operator =(const ThreadKit_var&);
    ~ThreadKit();

    ThreadKitRef operator ->() { return _obj_; }

    operator ThreadKit_in() const { return _obj_; }
    operator BaseThreadsObj() const;
    static ThreadKitRef _narrow(BaseObjectRef p);
    static ThreadKit_tmp _narrow(const BaseObject& r);

    static ThreadKitRef _duplicate(ThreadKitRef obj);
    static ThreadKit_tmp _duplicate(const ThreadKit& r);
};

class ThreadKit_tmp : public ThreadKit {
public:
    ThreadKit_tmp(ThreadKitRef p) { _obj_ = p; }
    ThreadKit_tmp(const ThreadKit& r);
    ThreadKit_tmp(const ThreadKit_tmp& r);
    ~ThreadKit_tmp();
};

class ThreadKit_var {
public:
    ThreadKitRef _obj_;

    ThreadKit_var(ThreadKitRef p) { _obj_ = p; }
    operator ThreadKitRef() const { return _obj_; }
    ThreadKitRef operator ->() { return _obj_; }
};

class ThreadKitType : public BaseThreadsObjType {
protected:
    ThreadKitType();
    virtual ~ThreadKitType();
public:
    ThreadObj_tmp thread(Action_in a) {
        return _c_thread(a);
    }
    virtual ThreadObjRef _c_thread(Action_in a) = 0;
    LockObj_tmp lock() {
        return _c_lock();
    }
    virtual LockObjRef _c_lock() = 0;
    ConditionVariable_tmp condition() {
        return _c_condition();
    }
    virtual ConditionVariableRef _c_condition() = 0;
    Semaphore_tmp general_semaphore(Long count) {
        return _c_general_semaphore(count);
    }
    virtual SemaphoreRef _c_general_semaphore(Long count) = 0;
    Semaphore_tmp mutex_semaphore() {
        return _c_mutex_semaphore();
    }
    virtual SemaphoreRef _c_mutex_semaphore() = 0;
    Semaphore_tmp wait_semaphore() {
        return _c_wait_semaphore();
    }
    virtual SemaphoreRef _c_wait_semaphore() = 0;
    ThreadKitRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

inline BaseThreadsObjRef BaseThreadsObj::_duplicate(BaseThreadsObjRef obj) {
    return (BaseThreadsObjRef)_BaseObject__duplicate(obj, 0);
}
inline BaseThreadsObj& BaseThreadsObj::operator =(BaseThreadsObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = BaseThreadsObj::_duplicate(p);
    return *this;
}
inline BaseThreadsObj::BaseThreadsObj(const BaseThreadsObj& r) {
    _obj_ = BaseThreadsObj::_duplicate(r._obj_);
}
inline BaseThreadsObj& BaseThreadsObj::operator =(const BaseThreadsObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = BaseThreadsObj::_duplicate(r._obj_);
    return *this;
}
inline BaseThreadsObj::BaseThreadsObj(const BaseThreadsObj_tmp& r) {
    _obj_ = r._obj_;
    ((BaseThreadsObj_tmp*)&r)->_obj_ = 0;
}
inline BaseThreadsObj& BaseThreadsObj::operator =(const BaseThreadsObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((BaseThreadsObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline BaseThreadsObj::BaseThreadsObj(const BaseThreadsObj_var& e) {
    _obj_ = BaseThreadsObj::_duplicate(e._obj_);
}
inline BaseThreadsObj& BaseThreadsObj::operator =(const BaseThreadsObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = BaseThreadsObj::_duplicate(e._obj_);
    return *this;
}
inline BaseThreadsObj::~BaseThreadsObj() {
    _BaseObject__release(_obj_);
}
inline BaseThreadsObj_tmp BaseThreadsObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline BaseThreadsObj_tmp BaseThreadsObj::_duplicate(const BaseThreadsObj& r) {
    return _duplicate(r._obj_);
}
inline BaseThreadsObj_tmp::BaseThreadsObj_tmp(const BaseThreadsObj& r) {
    _obj_ = BaseThreadsObj::_duplicate(r._obj_);
}
inline BaseThreadsObj_tmp::BaseThreadsObj_tmp(const BaseThreadsObj_tmp& r) {
    _obj_ = r._obj_;
    ((BaseThreadsObj_tmp*)&r)->_obj_ = 0;
}
inline BaseThreadsObj_tmp::~BaseThreadsObj_tmp() { }

inline ThreadObjRef ThreadObj::_duplicate(ThreadObjRef obj) {
    return (ThreadObjRef)_BaseObject__duplicate(obj, 0);
}
inline ThreadObj& ThreadObj::operator =(ThreadObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = ThreadObj::_duplicate(p);
    return *this;
}
inline ThreadObj::ThreadObj(const ThreadObj& r) {
    _obj_ = ThreadObj::_duplicate(r._obj_);
}
inline ThreadObj& ThreadObj::operator =(const ThreadObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = ThreadObj::_duplicate(r._obj_);
    return *this;
}
inline ThreadObj::ThreadObj(const ThreadObj_tmp& r) {
    _obj_ = r._obj_;
    ((ThreadObj_tmp*)&r)->_obj_ = 0;
}
inline ThreadObj& ThreadObj::operator =(const ThreadObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((ThreadObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline ThreadObj::ThreadObj(const ThreadObj_var& e) {
    _obj_ = ThreadObj::_duplicate(e._obj_);
}
inline ThreadObj& ThreadObj::operator =(const ThreadObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = ThreadObj::_duplicate(e._obj_);
    return *this;
}
inline ThreadObj::~ThreadObj() {
    _BaseObject__release(_obj_);
}
inline ThreadObj_tmp ThreadObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline ThreadObj_tmp ThreadObj::_duplicate(const ThreadObj& r) {
    return _duplicate(r._obj_);
}
inline ThreadObj::operator BaseThreadsObj() const {
    return BaseThreadsObj_tmp((BaseThreadsObjRef)_BaseObject__duplicate((BaseThreadsObjRef)_obj_, 0));
}
inline ThreadObj_tmp::ThreadObj_tmp(const ThreadObj& r) {
    _obj_ = ThreadObj::_duplicate(r._obj_);
}
inline ThreadObj_tmp::ThreadObj_tmp(const ThreadObj_tmp& r) {
    _obj_ = r._obj_;
    ((ThreadObj_tmp*)&r)->_obj_ = 0;
}
inline ThreadObj_tmp::~ThreadObj_tmp() { }

inline LockObjRef LockObj::_duplicate(LockObjRef obj) {
    return (LockObjRef)_BaseObject__duplicate(obj, 0);
}
inline LockObj& LockObj::operator =(LockObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = LockObj::_duplicate(p);
    return *this;
}
inline LockObj::LockObj(const LockObj& r) {
    _obj_ = LockObj::_duplicate(r._obj_);
}
inline LockObj& LockObj::operator =(const LockObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = LockObj::_duplicate(r._obj_);
    return *this;
}
inline LockObj::LockObj(const LockObj_tmp& r) {
    _obj_ = r._obj_;
    ((LockObj_tmp*)&r)->_obj_ = 0;
}
inline LockObj& LockObj::operator =(const LockObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((LockObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline LockObj::LockObj(const LockObj_var& e) {
    _obj_ = LockObj::_duplicate(e._obj_);
}
inline LockObj& LockObj::operator =(const LockObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = LockObj::_duplicate(e._obj_);
    return *this;
}
inline LockObj::~LockObj() {
    _BaseObject__release(_obj_);
}
inline LockObj_tmp LockObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline LockObj_tmp LockObj::_duplicate(const LockObj& r) {
    return _duplicate(r._obj_);
}
inline LockObj::operator BaseThreadsObj() const {
    return BaseThreadsObj_tmp((BaseThreadsObjRef)_BaseObject__duplicate((BaseThreadsObjRef)_obj_, 0));
}
inline LockObj_tmp::LockObj_tmp(const LockObj& r) {
    _obj_ = LockObj::_duplicate(r._obj_);
}
inline LockObj_tmp::LockObj_tmp(const LockObj_tmp& r) {
    _obj_ = r._obj_;
    ((LockObj_tmp*)&r)->_obj_ = 0;
}
inline LockObj_tmp::~LockObj_tmp() { }

inline ConditionVariableRef ConditionVariable::_duplicate(ConditionVariableRef obj) {
    return (ConditionVariableRef)_BaseObject__duplicate(obj, 0);
}
inline ConditionVariable& ConditionVariable::operator =(ConditionVariableRef p) {
    _BaseObject__release(_obj_);
    _obj_ = ConditionVariable::_duplicate(p);
    return *this;
}
inline ConditionVariable::ConditionVariable(const ConditionVariable& r) {
    _obj_ = ConditionVariable::_duplicate(r._obj_);
}
inline ConditionVariable& ConditionVariable::operator =(const ConditionVariable& r) {
    _BaseObject__release(_obj_);
    _obj_ = ConditionVariable::_duplicate(r._obj_);
    return *this;
}
inline ConditionVariable::ConditionVariable(const ConditionVariable_tmp& r) {
    _obj_ = r._obj_;
    ((ConditionVariable_tmp*)&r)->_obj_ = 0;
}
inline ConditionVariable& ConditionVariable::operator =(const ConditionVariable_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((ConditionVariable_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline ConditionVariable::ConditionVariable(const ConditionVariable_var& e) {
    _obj_ = ConditionVariable::_duplicate(e._obj_);
}
inline ConditionVariable& ConditionVariable::operator =(const ConditionVariable_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = ConditionVariable::_duplicate(e._obj_);
    return *this;
}
inline ConditionVariable::~ConditionVariable() {
    _BaseObject__release(_obj_);
}
inline ConditionVariable_tmp ConditionVariable::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline ConditionVariable_tmp ConditionVariable::_duplicate(const ConditionVariable& r) {
    return _duplicate(r._obj_);
}
inline ConditionVariable::operator BaseThreadsObj() const {
    return BaseThreadsObj_tmp((BaseThreadsObjRef)_BaseObject__duplicate((BaseThreadsObjRef)_obj_, 0));
}
inline ConditionVariable_tmp::ConditionVariable_tmp(const ConditionVariable& r) {
    _obj_ = ConditionVariable::_duplicate(r._obj_);
}
inline ConditionVariable_tmp::ConditionVariable_tmp(const ConditionVariable_tmp& r) {
    _obj_ = r._obj_;
    ((ConditionVariable_tmp*)&r)->_obj_ = 0;
}
inline ConditionVariable_tmp::~ConditionVariable_tmp() { }

inline SemaphoreRef Semaphore::_duplicate(SemaphoreRef obj) {
    return (SemaphoreRef)_BaseObject__duplicate(obj, 0);
}
inline Semaphore& Semaphore::operator =(SemaphoreRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Semaphore::_duplicate(p);
    return *this;
}
inline Semaphore::Semaphore(const Semaphore& r) {
    _obj_ = Semaphore::_duplicate(r._obj_);
}
inline Semaphore& Semaphore::operator =(const Semaphore& r) {
    _BaseObject__release(_obj_);
    _obj_ = Semaphore::_duplicate(r._obj_);
    return *this;
}
inline Semaphore::Semaphore(const Semaphore_tmp& r) {
    _obj_ = r._obj_;
    ((Semaphore_tmp*)&r)->_obj_ = 0;
}
inline Semaphore& Semaphore::operator =(const Semaphore_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Semaphore_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Semaphore::Semaphore(const Semaphore_var& e) {
    _obj_ = Semaphore::_duplicate(e._obj_);
}
inline Semaphore& Semaphore::operator =(const Semaphore_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Semaphore::_duplicate(e._obj_);
    return *this;
}
inline Semaphore::~Semaphore() {
    _BaseObject__release(_obj_);
}
inline Semaphore_tmp Semaphore::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Semaphore_tmp Semaphore::_duplicate(const Semaphore& r) {
    return _duplicate(r._obj_);
}
inline Semaphore::operator BaseThreadsObj() const {
    return BaseThreadsObj_tmp((BaseThreadsObjRef)_BaseObject__duplicate((BaseThreadsObjRef)_obj_, 0));
}
inline Semaphore_tmp::Semaphore_tmp(const Semaphore& r) {
    _obj_ = Semaphore::_duplicate(r._obj_);
}
inline Semaphore_tmp::Semaphore_tmp(const Semaphore_tmp& r) {
    _obj_ = r._obj_;
    ((Semaphore_tmp*)&r)->_obj_ = 0;
}
inline Semaphore_tmp::~Semaphore_tmp() { }

inline ThreadKitRef ThreadKit::_duplicate(ThreadKitRef obj) {
    return (ThreadKitRef)_BaseObject__duplicate(obj, 0);
}
inline ThreadKit& ThreadKit::operator =(ThreadKitRef p) {
    _BaseObject__release(_obj_);
    _obj_ = ThreadKit::_duplicate(p);
    return *this;
}
inline ThreadKit::ThreadKit(const ThreadKit& r) {
    _obj_ = ThreadKit::_duplicate(r._obj_);
}
inline ThreadKit& ThreadKit::operator =(const ThreadKit& r) {
    _BaseObject__release(_obj_);
    _obj_ = ThreadKit::_duplicate(r._obj_);
    return *this;
}
inline ThreadKit::ThreadKit(const ThreadKit_tmp& r) {
    _obj_ = r._obj_;
    ((ThreadKit_tmp*)&r)->_obj_ = 0;
}
inline ThreadKit& ThreadKit::operator =(const ThreadKit_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((ThreadKit_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline ThreadKit::ThreadKit(const ThreadKit_var& e) {
    _obj_ = ThreadKit::_duplicate(e._obj_);
}
inline ThreadKit& ThreadKit::operator =(const ThreadKit_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = ThreadKit::_duplicate(e._obj_);
    return *this;
}
inline ThreadKit::~ThreadKit() {
    _BaseObject__release(_obj_);
}
inline ThreadKit_tmp ThreadKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline ThreadKit_tmp ThreadKit::_duplicate(const ThreadKit& r) {
    return _duplicate(r._obj_);
}
inline ThreadKit::operator BaseThreadsObj() const {
    return BaseThreadsObj_tmp((BaseThreadsObjRef)_BaseObject__duplicate((BaseThreadsObjRef)_obj_, 0));
}
inline ThreadKit_tmp::ThreadKit_tmp(const ThreadKit& r) {
    _obj_ = ThreadKit::_duplicate(r._obj_);
}
inline ThreadKit_tmp::ThreadKit_tmp(const ThreadKit_tmp& r) {
    _obj_ = r._obj_;
    ((ThreadKit_tmp*)&r)->_obj_ = 0;
}
inline ThreadKit_tmp::~ThreadKit_tmp() { }

#endif
