#include <stdio.h>
#include <stdlib.h>
#include <jasper.h>

#define	FAILED	2

int main(int argc, char **argv)
{
	char *refpath;
	FILE *reffile;
	char *othpath;
	FILE *othfile;
	int c;
	jas_seq2d_t *x;
	jas_seq2d_t *y;

	refpath = 0;
	othpath = 0;

	while ((c = getopt(argc, argv, "f:F:")) != EOF) {
		switch (c) {
		case 'f':
			refpath = optarg;
			break;
		case 'F':
			othpath = optarg;
			break;
		}
	}

	if (!refpath || !othpath) {
		fprintf(stderr, "usage: %s -f reffile -F othfile\n", argv[0]);
		exit(FAILED);
	}

	if (!(reffile = fopen(refpath, "r"))) {
		fprintf(stderr, "cannot open %s\n", refpath);
		exit(FAILED);
	}
	if (!(othfile = fopen(othpath, "r"))) {
		fprintf(stderr, "cannot open %s\n", othpath);
		exit(FAILED);
	}

	if (!(x = jas_seq2d_input(reffile))) {
		fprintf(stderr, "cannot input reference\n");
		exit(FAILED);
	}
	if (!(y = jas_seq2d_input(othfile))) {
		fprintf(stderr, "cannot input other\n");
		exit(FAILED);
	}

	if (!jas_matrix_cmp(x, y)) {
		fprintf(stderr, "equal\n");
		exit(0);
	} else {
		fprintf(stderr, "not equal\n");
		exit(1);
	}

	exit(FAILED);

}

