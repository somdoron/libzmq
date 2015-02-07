/*:
    Copyright (c) 2007-2015 Contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "thread.hpp"
#include "testutil.hpp"

void worker1(void* s);
void worker2(void* s);

int main (void)
{
    setup_test_environment();
    void *ctx = zmq_ctx_new ();
    assert (ctx);

    void *client = zmq_socket (ctx, ZMQ_CLIENT);
    void *client2 = zmq_socket (ctx, ZMQ_CLIENT);

    int rc;

    rc = zmq_bind (client, "tcp://127.0.0.1:5560");
    assert (rc == 0);

    rc = zmq_connect (client2, "tcp://127.0.0.1:5560");
    assert (rc == 0);

	zmq::thread_t t1;
	t1.start(worker1, client2);

	zmq::thread_t t2;
	t2.start(worker2, client2);	

	char data[1];
	data[0] = 0;

	for (int i=0; i < 10; i++) {
		rc = zmq_send_const(client, data, 1, 0);
		assert (rc == 1);

		rc = zmq_send_const(client, data, 1, 0);
		assert(rc == 1);

		char a, b;

		rc = zmq_recv(client, &a, 1, 0);
		assert(rc == 1);

		rc = zmq_recv(client, &b, 1, 0);
		assert(rc == 1);

		printf("%d %d\n", (int)a, (int)b);

		// make sure they came from different threads
		assert((a == 1 && b == 2) || (a == 2 && b == 1));
	}

	// make the thread exit
	data[0] = 1;

	rc = zmq_send_const(client, data, 1, 0);
	assert (rc == 1);

	rc = zmq_send_const(client, data, 1, 0);
	assert(rc == 1);

	t1.stop();
	t2.stop();

    rc = zmq_close (client2);
    assert (rc == 0);

    rc = zmq_close (client);
    assert (rc == 0);

    rc = zmq_ctx_term (ctx);
    assert (rc == 0);

    return 0 ;
}

void worker1(void* s)
{
	const char worker_id = 1;
	char c;

	while (true)
	{
		int rc = zmq_recv(s, &c,1, 0); 
		assert(rc == 1);

		if (c == 0)
		{
			// emulate some work
			printf("one took\n");
			sleep(1);			
			rc = zmq_send_const(s,&worker_id, 1, 0);
			assert(rc == 1);
		}
		else
		{
			// we got exit request
			break;
		}
	}
}

void worker2(void* s)
{
	const char worker_id = 2;
	char c;

	while (true)
	{
		int rc = zmq_recv(s, &c,1, 0); 
		assert(rc == 1);

		assert(c == 1 || c == 0);

		if (c == 0)
		{
			// emulate some work
			printf("two took\n");
			sleep(1);			
			rc = zmq_send_const(s,&worker_id, 1, 0);
			assert(rc == 1);
		}
		else
		{
			// we got exit request
			break;
		}
	}
}






