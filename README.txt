PURPOSE:

The purpose of this lab is to construct a multi-threaded web server using POSIX threads (pthreads) in C language to learn about thread programming and synchronization methods. Our web server will be able to handle any file type: HTML, GIF, JPEG, TXT, etc. and of any arbitrary size. It should handle a limited portion of the HTTP web protocol (namely, the GET command to fetch a web page / files).

HOW TO USE:

Extract "testing.tar" and use it as your web server root directory. Run your web server using the following command

./web_server <port> <path_to_testing>/testing <num_dispatch> <num_worker> <dynamic_flag> <queue_len> <cache_entries>

**** Pick a random number other than 9000 from (1024 to 65536) to avoid collisions with other groups ****
For example, to run the web server at port 9000, with root directory "/home/student/joe/testing" with 100 dispatch and worker threads,
queue length 100 run the following command

./web_server 9000 /home/student/joe/testing 100 100 0 100 100

You should now (using another terminal) be able to query a single file, such as the following:

wget http://127.0.0.1:9000/image/jpg/29.jpg

If you have a file containing all the URLs you want, you can open a terminal and issue the following command:

wget -i <path-to-urls>/urls -O results

The above command will ask wget to fetch all the URLs listed on the file named "urls" that you downloaded from the assignment page.

To run the xargs test (test concurrency):

cat <path _to_urls_file> | xargs -n 1 -P 8 wget

HOW IT WORKS

Our server will be composed of two types of threads: dispatcher threads and worker threads. The purpose of the dispatcher threads is to repeatedly accept an incoming connection, read the client request from the connection, and place the request in a queue. We will assume that there will only be one request per incoming connection. The purpose of the worker threads is to monitor the request queue, retrieve requests and serve the request’s result back to the client. The request queue is a bounded buffer and is properly synchronized using CVs.
