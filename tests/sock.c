//
// Copyright 2016 Garrett D'Amore <garrett@damore.org>
//
// This software is supplied under the terms of the MIT License, a
// copy of which should be located in the distribution where this
// file was obtained (LICENSE.txt).  A copy of the license may also be
// found online at https://opensource.org/licenses/MIT.
//

#include "convey.h"
#include "nng.h"

TestMain("Socket Operations", {
	Convey("We are able to open a PAIR socket", {
		int rv;
		nng_socket *sock = NULL;

		rv = nng_open(&sock, NNG_PROTO_PAIR);
		So(rv == 0);
		So(sock != NULL);

		Convey("And we can close it", {
			rv = nng_close(sock);
			So(rv == 0);
		})

		Convey("It's type is still proto", {
			So(nng_protocol(sock) == NNG_PROTO_PAIR);
		})

		Convey("Recv with no pipes times out correctly", {
			nng_msg *msg = NULL;
			int64_t when = 500000;
			uint64_t now;

			// We cheat to get access to the core's clock.
			extern uint64_t nni_clock(void);
			now = nni_clock();

			rv = nng_setopt(sock, NNG_OPT_RCVTIMEO, &when,
				sizeof (when));
			So(rv == 0);
			rv = nng_recvmsg(sock, &msg, 0);
			So(rv == NNG_ETIMEDOUT);
			So(msg == NULL);
			So(nni_clock() > (now + 500000));
			So(nni_clock() < (now + 1000000));
		})

		Convey("Recv nonblock with no pipes gives EAGAIN", {
			nng_msg *msg = NULL;
			rv = nng_recvmsg(sock, &msg, NNG_FLAG_NONBLOCK);
			So(rv == NNG_EAGAIN);
			So(msg == NULL);
		})

		Convey("We can set and get options", {
			int64_t when = 1234;
			int64_t check = 0;
			size_t sz;
			rv = nng_setopt(sock, NNG_OPT_SNDTIMEO, &when,
				sizeof (when));
			So(rv == 0);
			sz = sizeof (check);
			Convey("Short size is not copied", {
				sz = 0;
				rv = nng_getopt(sock, NNG_OPT_SNDTIMEO,
					&check, &sz);
				So(rv == 0);
				So(sz == sizeof (check));
				So(check == 0);
			})
			Convey("Correct size is copied", {
				sz = sizeof (check);
				rv = nng_getopt(sock, NNG_OPT_SNDTIMEO, &check,
					&sz);
				So(rv == 0);
				So(sz == sizeof (check));
				So(check == 1234);
			})
		})
	})
})