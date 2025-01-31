#include "fib.decl.h"

#include <ckcallback.h>

int THRESHOLD = 2; 

class ValueMsg: public CMessage_ValueMsg {
    public:   
        long value; 
};

class Main : public CBase_Main {
    public:
        Main(CkMigrateMessage *m) {};
        Main(CkArgMsg* m) { thisProxy.run(atoi(m->argv[1])); }
        void run(long n) {
            CkFuture f = CkCreateFuture();
            CProxy_Fib::ckNew(n, f);
            ValueMsg *m = (ValueMsg*)CkWaitFuture(f);
            CkPrintf("The requested Fibonacci number is : %ld\n", m->value);
            CkExit();
        }
};

class Fib : public CBase_Fib {
    public:
        long result;
        Fib(CkMigrateMessage *m) {};
        Fib(long n, CkFuture f){ 
            thisProxy.run(n, f); 
        }

        void run(long n, CkFuture f) {
            if (n < THRESHOLD) {
                result = seqFib(n);
            } else {

                CkEntryOptions opts; 
                opts.setQueueing(CK_QUEUEING_LIFO); 

                CkFuture f1 = CkCreateFuture();
                CkFuture f2 = CkCreateFuture();
                CProxy_Fib::ckNew(n-1, f1, CK_PE_ANY, &opts);
                CProxy_Fib::ckNew(n-2, f2, CK_PE_ANY, &opts);
                ValueMsg* m1 = (ValueMsg*)CkWaitFuture(f1);
                ValueMsg* m2 = (ValueMsg*)CkWaitFuture(f2);
                result = m1->value + m2->value;
                delete m1; 
                delete m2;
                CkReleaseFuture(f1);
                CkReleaseFuture(f2);
            }
            ValueMsg *m = new ValueMsg();
            m->value = result;
            CkSendToFuture(f, m);

            delete this; 
        }

        long seqFib(long n) {
            if (n <= 1) {
                return n;
            }
            return seqFib(n - 1) + seqFib(n - 2);
        }
};

#include "fib.def.h"