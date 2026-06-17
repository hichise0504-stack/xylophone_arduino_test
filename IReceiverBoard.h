#ifndef IRECEIVERBOARD_H
#define IRECEIVERBOARD_H


class IReceiverBoard {
public:
virtual ~IReceiverBoard() {}
virtual void begin() = 0;
virtual bool read() = 0;
};


#endif
