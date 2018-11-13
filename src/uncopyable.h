/*
 * Uncopyable.h
 */

#ifndef UNCOPYABLE_H_
#define UNCOPYABLE_H_

class UnCopyable
{
public:
    UnCopyable() {}
    ~UnCopyable() {}
private:
    UnCopyable(const UnCopyable&);
    UnCopyable(UnCopyable&&);
    UnCopyable& operator=(const UnCopyable&);
    UnCopyable& operator=(UnCopyable&&);
};

#endif /* UNCOPYABLE_H_ */
