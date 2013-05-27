#include <jasper/jasper.h>

int main(int argc, char **argv)
{
	int i;
	jas_tmr_t tmr;
	jas_tmr_t dummytmr;
	double t;
	int numiters;

	if (argc < 2) {
		fprintf(stderr, "bad usage :P\n");
		exit(1);
	}

	numiters = atoi(argv[1]);

	jas_tmr_start(&tmr);
	for (i = numiters; i > 0; --i) {
		jas_tmr_start(&dummytmr);
	}
	jas_tmr_stop(&tmr);
	t = jas_tmr_get(&tmr);
	t /= numiters;
	printf("jas_tmr_start %.3f us\n", t * 1e6);

	jas_tmr_start(&tmr);
	for (i = numiters; i > 0; --i) {
		jas_tmr_stop(&dummytmr);
	}
	jas_tmr_stop(&tmr);
	t = jas_tmr_get(&tmr);
	t /= numiters;
	printf("jas_tmr_stop  %.3f us\n", t * 1e6);

	t = 0;
	for (i = numiters; i > 0; --i) {
		jas_tmr_start(&tmr);
		jas_tmr_stop(&tmr);
		t += jas_tmr_get(&tmr);
	}
	t /= numiters;
	printf("zero time %.3f us\n", t * 1e6);

	jas_tmr_start(&tmr);
	sleep(1);
	jas_tmr_stop(&tmr);
	t = jas_tmr_get(&tmr);
	printf("time delay %.8f s\n", t);

	exit(0);
}
