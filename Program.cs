using System;
using System.Threading;

namespace ConsoleApp1
{
    class Program
    {
        static int[] data;
        static ReaderWriterLock rwl = new ReaderWriterLock();
        static readonly int time_ms = 1000;
        static int wait = 0;

        static void Writer(object n)
        {
            rwl.AcquireWriterLock(time_ms);
            for (int i = 0; i < data.Length; ++i)
            {
                data[i] = (int)n;
            }
            rwl.ReleaseWriterLock();
            Interlocked.Decrement(ref wait);
        }

        static void Reader(object n)
        {
            rwl.AcquireReaderLock(time_ms);
            int v = data[0];
            for (int i = 0; i < data.Length; ++i)
            {
                if (v != data[i])
                {
                    Console.WriteLine("ERROR: data are inconsistent: the first value is " + v + " incorrect value is" + data[i]);
                    break;
                }
            }
            rwl.ReleaseReaderLock();
            Interlocked.Decrement(ref wait);
        }

        static void ViaPool()
        {
            int n = 100000;
            data = new int[n];
            for (int i = 0; i < n; ++i)
            {
                data[i] = 0;
            }

            int taskCount = 100;
            wait = taskCount;
            for (int i = 0; i < taskCount; ++i)
            {
                if (i % 2 == 0)
                {
                    ThreadPool.QueueUserWorkItem(Reader, i);
                }
                else
                {
                    ThreadPool.QueueUserWorkItem(Writer, i);
                }
            }

            while (0 != wait)
            {
                Thread.Sleep(1);
            }
            Console.WriteLine("Done");
            Console.ReadKey();
        }

        static void ViaThread()
        {
            int n = 100000;
            data = new int[n];
            for (int i = 0; i < n; ++i)
            {
                data[i] = 0;
            }

            int taskCount = 100;
            wait = taskCount;
            Thread[] my_threads = new Thread[taskCount];
            for (int i = 0; i < taskCount; ++i)
            {
                if (i % 2 == 0)
                {
                    my_threads[i] = new Thread(Reader);
                    my_threads[i].Start(i);
                }
                else
                {
                    my_threads[i] = new Thread(Writer);
                    my_threads[i].Start(i);
                }
            }

            for (int i = 0; i < taskCount; ++i)
            {
                my_threads[i].Join();
            }
            Console.WriteLine("Done");
            Console.ReadKey();
        }

        static void Main1(string[] args)
        {
            ViaThread();
        }
    }

    class Program2
    {
        static int wait;
        static readonly int m = 200, l = 300, n = 400;
        static int[,] a = new int[l, m];
        // it is bad go to columns let's T-1
        static int[,] b = new int[n, m];
        static int[,] r = new int[l, n];
        static Random rand = new Random();

        static void CalculateCell(object state)
        {
            object[] array = state as object[];
            int y = Convert.ToInt32(array[0]);
            int x = Convert.ToInt32(array[1]);

            int v = 0;
            for (int i = 0; i < m; ++i)
            {
                v += (a[y, i] * b[x, i]);
            }
            r[y, x] = v;
            Interlocked.Decrement(ref wait);
        }

        static void Sync()
        {
            for (int i = 0; i < l; ++i)
            {
                for (int j = 0; j < n; ++j)
                {
                    CalculateCell(new object[] { i, j });
                }
            }
        }

        static void Async()
        {
            wait = l * n;
            for (int i = 0; i < l; ++i)
            {
                for (int j = 0; j < n; ++j)
                {
                    ThreadPool.QueueUserWorkItem(CalculateCell, new object[] { i, j });
                }
            }
            while (0 != wait)
            {
                Thread.Sleep(1);
            }
        }

        static void Generate(ref int[,] v, int x, int y)
        {
            for (int i = 0; i < y; ++i)
            {
                for (int j = 0; j < x; ++j)
                {
                    v[i, j] = rand.Next(1, 12);
                    //Console.Write(v[i, j] + ", ");
                }
                //Console.WriteLine();
            }
            //Console.WriteLine();
        }

        static void Main(string[] args)
        {
            Generate(ref a, m, l);
            Generate(ref b, m, n);

            DateTime start_sync;
            DateTime stop_sync;

            DateTime start_async;
            DateTime stop_async;

            Sync();

            int c = 100;
            start_async = DateTime.Now;
            for (int i = 0; i < c; ++i)
            {
                Async();
            }
            stop_async = DateTime.Now;

            start_sync = DateTime.Now;
            for (int i = 0; i < c; ++i)
            {
                Sync();
            }
            stop_sync = DateTime.Now;

            /*
            for (int i = 0; i < l; ++i)
            {
                for (int j = 0; j < n; ++j)
                {
                    Console.Write(r[i, j] + ", ");
                }
                Console.WriteLine();
            }
            Console.WriteLine();
            */

            TimeSpan elapsed_sync = stop_sync.Subtract(start_sync);
            TimeSpan elapsed_async = stop_async.Subtract(start_async);

            Console.WriteLine("Sync time: " + elapsed_sync);
            Console.WriteLine("Async time: " + elapsed_async);

            // Sync time: 00:00:08.8624509
            // Async time: 00:00:05.8214114

            Console.ReadKey();
        }
    }
}
