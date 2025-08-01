package sd.producerConsumer.method3;

/**
 *    General description:
 *       definition of a FIFO memory of generic objects built in as a monitor.
 *       It extends a generic memory data type.
 */

public class MemFIFO<R> extends MemObject<R>
{
  /**
   *  Characterization of the FIFO access discipline
   */

   private int inPnt,                                      // insertion pointer
               outPnt;                                     // retrieval pointer
   private boolean empty;                                  // signaling empty FIFO

  /**
   *  Constructor
   *
   *    @param storage storage area
   */

   public MemFIFO (R [] storage)
   {
     super (storage);
     inPnt = outPnt = 0;
     empty = true;
   }

  /**
   *  Writing a value in mutual exclusion regime.
   *
   *    @param val value to store
   */

   @Override
   public synchronized void write (R val)
   {
     while ((inPnt == outPnt) && !empty)
     { try
       { wait ();                                          // the calling thread must block if FIFO is fully occupied
       }
       catch (InterruptedException e) {}
     }

     mem[inPnt] = val;
     inPnt = (inPnt + 1) % mem.length;
     empty = false;
     notifyAll ();
   }

  /**
   *  Reading a value in mutual exclusion regime.
   *
   *    @return the retrieved value
   */

   @Override
   public synchronized R read ()
   {
     R val;                                                // retrieved value

     while (empty)
     { try
       { wait ();                                          // the calling thread must block if FIFO is empty
       }
       catch (InterruptedException e) {}
     }

     val = mem[outPnt];
     outPnt = (outPnt + 1) % mem.length;
     empty = (inPnt == outPnt);
     notifyAll ();

     return val;
   }
}
