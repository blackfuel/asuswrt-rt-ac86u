#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libgen.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "shn_dc.h"

#undef DBG
#define DBG(fmt, args...) 	printf("[%s(%d)]: "fmt, __func__, __LINE__, ##args)
//#define DBG(fmt, args...)

#undef ERR
#define ERR(fmt, args...) fprintf(stderr, fmt, ##args)

#define MAX_ADDR_STR_SIZ	64
#define MAX_DATA_STR_SIZ	128

#define ARRAY_SIZE(__a)	(sizeof(__a) / sizeof(__a[0]))

static struct {
	int iter;
} prog_conf = {
	.iter = 1
};

static uint8_t alnum[] =
	"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static int nr_alnum = sizeof(alnum);

static void gen_rand_addr_str(uint8_t *buff, int buff_len)
{
	int i;

	int at_pos = rand() % buff_len;
	int dot_pos = at_pos + (rand() % (buff_len - at_pos));

	for (i = 0; i < buff_len-1; i++)
	{
		if (at_pos == i)
		{
			buff[i] = '@';
		}
		else if (dot_pos == i)
		{
			buff[i] = '.';
		}
		else
		{
			buff[i] = alnum[rand() % (nr_alnum - 1)];
		}
	}

	buff[i] = 0;	/* NULL-term'ed */
}

static int test_set_fw_ver(void)
{
	char *ver = "ABC-1234";
	return shn_dc_set_fw_ver(ver);
}

static int test_set_email(void)
{
	int i;

	uint8_t *email[SHN_DC_MAX_NR_MAIL] = { 0 };

	int ret;

	for (i = 0; i < SHN_DC_MAX_NR_MAIL; i++)
	{
		assert((email[i] = malloc(MAX_ADDR_STR_SIZ)));
		gen_rand_addr_str(email[i], MAX_ADDR_STR_SIZ);
		DBG("email#%d: %s\n", i, email[i]);
	}

	ret = shn_dc_set_email(email, SHN_DC_MAX_NR_MAIL);

	for (i--; i >= 0; i--)
	{
		free(email[i]);
	}

	return ret;
}

static int test_set_eula(void)
{
	int eula;

	int ret;

	eula = rand();
	DBG("eula=%d\n", eula);
	ret = shn_dc_set_eula(eula);

	return ret;
}

static int test_set_proxy_info(void)
{
	int i;

	proxy_info_t *pi;

	int ret;

	assert((pi = malloc(sizeof(proxy_info_t) * SHN_DC_MAX_NR_PROXY_INFO)));

	for (i = 0; i < SHN_DC_MAX_NR_PROXY_INFO; i++)
	{
#define PROXY_TYPE_MASK (PROXY_TYPE_SOCK4|PROXY_TYPE_SOCK4|PROXY_TYPE_HTTP)
		pi[i].type = rand() & PROXY_TYPE_MASK;
		if (!pi[i].type)
		{
			pi[i].type = PROXY_TYPE_SOCK4;
		}
		assert((pi[i].proxy = malloc(MAX_ADDR_STR_SIZ)));
		assert((pi[i].extra = malloc(MAX_DATA_STR_SIZ)));

		gen_rand_addr_str(pi[i].proxy, MAX_ADDR_STR_SIZ);
		gen_rand_addr_str(pi[i].extra, MAX_DATA_STR_SIZ);
		DBG("proxy#%d: %s, extra=%s\n", i, pi[i].proxy, pi[i].extra);
	}

	ret = shn_dc_set_proxy_info(pi, SHN_DC_MAX_NR_PROXY_INFO);

	for (i--; i >= 0; i--)
	{
		free(pi[i].proxy);
		free(pi[i].extra);
	}
	free(pi);

	return ret;
}

static int test_set_isp(void)
{
	int i;

	uint8_t* isp[SHN_DC_MAX_NR_ISP];

	int ret;

	for (i = 0; i < SHN_DC_MAX_NR_ISP; i++)
	{
		assert((isp[i] = malloc(MAX_ADDR_STR_SIZ)));
		gen_rand_addr_str(isp[i], MAX_ADDR_STR_SIZ);
		DBG("isp#%d: %s\n", i, isp[i]);
	}

	ret = shn_dc_set_isp(isp, SHN_DC_MAX_NR_ISP);

	for (i--; i >= 0; i--)
	{
		free(isp[i]);
	}

	return ret;
}

static int test_set_hc_sec(void)
{
	int i;

	hc_sec_t *sec;

	int ret;

	assert((sec = malloc(sizeof(hc_sec_t) * SHN_DC_MAX_NR_HC_SEC)));
	for (i = 0; i < SHN_DC_MAX_NR_HC_SEC; i++)
	{
		sec[i].type = HC_SEC_NULL + (rand() % HC_SEC_MAX);
		if (HC_SEC_NULL == sec[i].type)
		{
			sec[i].type = HC_SEC_NULL + 1;
		}
		sec[i].value = rand() % 3;
		DBG("health check security #%d: type=%d, val=%d\n"
			, i, sec[i].type, sec[i].value);
	}

	ret = shn_dc_set_hc_sec(sec, SHN_DC_MAX_NR_HC_SEC);

	free(sec);

	return ret;
}

static int test_set_hc_serv(void)
{
	int i;

	hc_serv_t *serv;

	int ret;

	assert((serv = malloc(sizeof(hc_serv_t) * SHN_DC_MAX_NR_HC_SERV)));
	for (i = 0; i < SHN_DC_MAX_NR_HC_SERV; i++)
	{
		serv[i].type = HC_SERV_NULL + (rand() % HC_SERV_MAX);
		if (HC_SERV_NULL == serv[i].type)
		{
			serv[i].type = HC_SERV_NULL + 1;
		}
		serv[i].value = rand() % 3;
		DBG("health check service #%d: type=%d, val=%d\n"
			, i, serv[i].type, serv[i].value);
	}

	ret = shn_dc_set_hc_serv(serv, SHN_DC_MAX_NR_HC_SERV);

	free(serv);

	return ret;
}

static int test_set_hc_conf(void)
{
	int i;

	hc_conf_t *conf;

	int ret;

	char attr[16];

	assert((conf = malloc(sizeof(hc_conf_t) * SHN_DC_MAX_NR_HC_CONF)));

	for (i = 0; i < SHN_DC_MAX_NR_HC_CONF; i++)
	{
		conf[i].type = HC_CONF_NULL + (rand() % HC_CONF_MAX);
		if (HC_CONF_NULL == conf[i].type)
		{
			conf[i].type = HC_CONF_NULL + 1;
		}

		memset(attr, 0, 16);
		snprintf(attr, 15, "0x%x", rand());
		conf[i].attribute = strdup(attr);

		DBG("health check config #%d: type=%d, attr=%s\n"
			, i, conf[i].type, conf[i].attribute);
	}

	ret = shn_dc_set_hc_conf(conf, SHN_DC_MAX_NR_HC_CONF);

	for (i = 0; i < SHN_DC_MAX_NR_HC_CONF; i++)
	{
		if (conf[i].attribute)
		{
			free(conf[i].attribute);
		}
	}
	free(conf);

	return ret;
}

static int test_set_hc_wl_enc(void)
{
	int i;

	hc_wl_enc_t *wl_enc;

	int ret;

	static char *wl_if_name[] = {
		"eth1", "eth2", "eth3",
	};

	int wl_cnt = ARRAY_SIZE(wl_if_name);

	assert((wl_enc = malloc(sizeof(hc_wl_enc_t) * wl_cnt)));

	for (i = 0; i < wl_cnt; i++)
	{
		wl_enc[i].if_name = strdup(wl_if_name[i]);
		wl_enc[i].band = WL_BAND_NULL + 1 + (rand() % (WL_BAND_MAX - 1));
		wl_enc[i].enc_type = WL_ENC_TYPE_OPEN + (rand() % WL_ENC_TYPE_MAX);
		wl_enc[i].pwd_stren = WL_PWD_STREN_NONE + (rand() % WL_PWD_STREN_MAX);

		DBG("health check wl_enc #%d: if_name=%s, band=%d, enc_type=%d, pwd_stren=%d\n"
			, i, wl_enc[i].if_name, wl_enc[i].band,
			wl_enc[i].enc_type, wl_enc[i].pwd_stren);
	}

	ret = shn_dc_set_hc_wl_enc(wl_enc, wl_cnt);

	for (i = 0; i < wl_cnt; i++)
	{
		if (wl_enc[i].if_name)
		{
			free(wl_enc[i].if_name);
		}
	}

	free(wl_enc);

	return ret;
}


static void __attribute__((noreturn))
show_help(char *prog_name)
{
	printf("Usage: %s options\n", prog_name);
	printf("Options:\n");
	printf("  --iter,-i    Specify the test iteration, default=1\n");
	printf("  --help,-h    Show this help\n");

	exit(0);
}

static void parse_arg(int argc, char *argv[])
{
	char *opt_str = "i:";

	struct option opt[] =
	{
		{ "iter", required_argument, 0, 'i' },
		{ NULL, 0, 0, 0 }
	};

	int c, opt_idx;

	while (-1 != (c = getopt_long(argc, argv, opt_str, opt, &opt_idx)))
	{
		switch(c)
		{
			case 'i':
				prog_conf.iter = strtol(optarg, NULL, 10);
				break;

			default:
				show_help(basename(argv[0]));
				break;
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}

	DBG("iter=%d\n", prog_conf.iter);
}

int main(int argc, char *argv[])
{
	struct {
		char *t_api;
		int (*t_func)(void);
	} test[] = {
#define TETR(__n)	{ "shn_dc_"#__n"()", test_##__n }
		TETR(set_fw_ver),
//		TETR(set_bw),
		TETR(set_email),
		TETR(set_eula),
//		TETR(set_dpi_conf),
		TETR(set_proxy_info),
		TETR(set_isp),
		//TETR(set_sec_conf),
		TETR(set_hc_sec),
		TETR(set_hc_serv),
		TETR(set_hc_conf),
		TETR(set_hc_wl_enc),
	};

	int i, j;
	int nr_test = ARRAY_SIZE(test);

	struct timeval tv_beg, tv_end, tv_tot;
	struct timezone tz;
	double tot_usec;

	memset(&tv_tot, 0, sizeof(tv_tot));
	memset(&tz, 0, sizeof(tz));

	parse_arg(argc, argv);

	for (i = 0; i < prog_conf.iter; i++)
	{
		printf("\n*** iter#%d *********************************************************\n", i);
		/*
		 * init the random seed
		 */
		srand(time(NULL));

		gettimeofday(&tv_beg, &tz);
		for (j = 0; j < nr_test; j++)
		{
			int ret;

			ret = test[j].t_func();

			printf("test %s ... %s (code=%d)\n"
				, test[j].t_api, (SHN_DC_OK == ret) ? "OK" : "Failed", ret);
		}
		gettimeofday(&tv_end, &tz);

		timersub(&tv_end, &tv_beg, &tv_end);
		timeradd(&tv_tot, &tv_end, &tv_tot);
	}

	tot_usec = tv_tot.tv_sec * 1000000 + tv_tot.tv_usec;
	tot_usec /= prog_conf.iter;

	printf("\nelpased total time=%d.%d, avg time per iter=%d.%d\n"
		, (int)tv_tot.tv_sec, (int)tv_tot.tv_usec
		, (int)(tot_usec / 1000000), (int)tot_usec % 1000000);

	return 0;
}
