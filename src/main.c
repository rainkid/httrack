/* ------------------------------------------------------------ */
/*
 HTTrack Website Copier, Offline Browser for Windows and Unix
 Copyright (C) Xavier Roche and other contributors

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3
 of the License, or any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


 Important notes:

 - We hereby ask people using this source NOT to use it in purpose of grabbing
 emails addresses, or collecting any other private information on persons.
 This would disgrace our work, and spoil the many hours we spent on it.


 Please visit our Website: http://www.httrack.com
 */

/* ------------------------------------------------------------ */
/* File: httrack.c console progress info                        */
/* Only used on Linux & FreeBSD versions                        */
/* Author: rainkide@gmail.com                                   */
/* ------------------------------------------------------------ */

#ifndef _WIN32
#ifndef Sleep
#define Sleep(a) { if (((a)*1000)%1000000) usleep(((a)*1000)%1000000); if (((a)*1000)/1000000) sleep(((a)*1000)/1000000); }
#endif
#endif

#include "hts/httrack-library.h"
#include <pthread.h>
#include "hts/htsglobal.h"
#include "hts/htsbase.h"
#include "hts/htsopt.h"
#include "hts/htsdefines.h"
#include "hts/htslib.h"
#include "hts/htsalias.h"

#include "main.h"

/* specific definitions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>

#include "regex.h"

#include "html/htmlparser.h"

// libevent
#include <err.h>
#include <event.h>
#include <evhttp.h>

struct httpd_cfg {
	char *listen;
	int port;
	int timeout;
};

unsigned long pid = 0;

enum PRIORITY{
	PRIORITY_JUST_SCAN, //just scan, don't save anything (for checking links)
	PRIORITY_ONLY_HTML, //save only html files
	PRIORITY_NOT_HTML, //save only non html files
	PRIORITY_ALL_FILE, //save all files
	PRIORITY_HTML_TREAT, //get html files before, then treat other files
};

enum ACTION{
	ACTION_ALL_SITES, //w *mirror web sites (--mirror)
	ACTION_ONLY_FILES, //just get files (saved in the current directory) (--get-files)
	ACTION_WITH_CACHE,
	ACTION_FIST_LEVEL,//mirror ALL links located in the first level pages (mirror links) (--mirrorlinks)
};

enum CACHE{
	CACHE_WITHOUT, // go without a cache
	CACHE_PRIORITY, //do not check remotly if the file has been updated or not, just load the cache content
	CACHE_BEST_USE, //see what works best and use it (the default)
};

enum REBOT{
	REBOT_NEVER, //never
	REBOT_SOMETIMES, //sometimes
	REBOT_ALWAYS , //always
};

enum ABANDON {
	ABANDON_NEVER,  //never
	ABANDON_TIMEOUT, //a timeout is	reached
	ABANDON_SLOW, // slow traffic is detected
	ABANDON_TIMEOUT_AND_SLOW, //a timeout is reached OR	slow traffic is detected
};

struct hts_proj {
	char name[256];//proj name
	char urls[2048];//where mirror from
	char savepath[1024]; //download file savepath
	char proxy[256]; //proxy port
	char linklist[256]; //link in files
	char proxy_port[256]; //proxy port
	int depth; //limit the directlry depth do not max to 20
	int errorno; // errorno
	int timeout; //timeout, number of seconds after a non-responding link is shutdown
	int retries; //increases the number of retries
	int conns; // limits the number of conns per second default 10
	enum CACHE cache; //a cache memory area is used for updates and retries to make the process far more efficient than it would otherwise be
	enum REBOT rebot; //robots protocol
	enum PRIORITY priority; // mirror options
	enum ACTION action; //file type chose
	enum ABANDON abandon; // These three options will cause the download from ahost to be abandoned
	int argc;
	char *argv[2048];
};

char* argv[] = { NULL, NULL, NULL, NULL, NULL };
int argc = 0;

static httrackp *opt = NULL;
struct evhttp *httpd = NULL;

static void signal_handlers(void);
static void et_init(void);
static void et_wait(void);
static void et_uninit(void);
static void httpd_handler(struct evhttp_request *req, void *arg);
void new_dl_thread();

void ht_init(void);
void ht_uninit(void);

static void sig_ignore(int code) { // ignorer signal
}

static void sig_term(int code) { // quitter brutalement
	fprintf(stderr, "\nProgram terminated (signal %d)\n", code);
	exit(0);
}

static void sig_finish(int code) { // finir et quitter
	signal(code, sig_term); // quitter si encore
	if (opt != NULL) {
		opt->state.exit_xh = 1;
	}
	fprintf(stderr, "\nExit requested to engine (signal %d)\n", code);
}

static void sig_doback(int blind) { // mettre en backing
	int out = -1;
	//
	printf("\nMoving into background to complete the mirror...\n");
	fflush(stdout);

	if (opt != NULL) {
		// suppress logging and asking lousy questions
		opt->quiet = 1;
		opt->verbosedisplay = 0;
	}

	if (!blind)
		out = open("hts-nohup.out", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (out == -1)
		out = open("/dev/null", O_WRONLY, S_IRUSR | S_IWUSR);
	dup2(out, 0);
	dup2(out, 1);
	dup2(out, 2);

	switch (fork()) {
	case 0:
		break;
	case -1:
		fprintf(stderr, "Error: can not fork process\n");
		break;
	default: // pere
		_exit(0);
		break;
	}
}

static void sig_back(int code) { // ignorer et mettre en backing
	if (opt != NULL && !opt->background_on_suspend) {
		signal(SIGTSTP, SIG_DFL); // ^Z
		printf("\nInterrupting the program.\n");
		fflush(stdout);
		kill(getpid(), SIGTSTP);
	} else {
		// Background the process.
		signal(code, sig_ignore);
		sig_doback(0);
	}
}

static void sig_ask(int code) { // demander
	char s[256];
	signal(code, sig_term); // quitter si encore
	printf(
			"\nQuit program/Interrupt/Background/bLind background/Cancel? (Q/I/B/L/C) ");
	fflush(stdout);
	scanf("%s", s);
	if ((s[0] == 'y') || (s[0] == 'Y') || (s[0] == 'o') || (s[0] == 'O')
			|| (s[0] == 'q') || (s[0] == 'Q'))
		exit(0); // quitter
	else if ((s[0] == 'b') || (s[0] == 'B') || (s[0] == 'a') || (s[0] == 'A'))
		sig_doback(0); // arriere plan
	else if ((s[0] == 'l') || (s[0] == 'L'))
		sig_doback(1); // arriere plan
	else if ((s[0] == 'i') || (s[0] == 'I')) {
		if (opt != NULL) {
			// ask for stop
			printf("finishing pending transfers.. please wait\n");
			opt->state.stop = 1;
		}
		signal(code, sig_ask); // remettre signal
	} else {
		printf("cancel..\n");
		signal(code, sig_ask); // remettre signal
	}
}
static void sig_brpipe(int code) { // treat if necessary
	signal(code, sig_brpipe);
}

static void sig_leave(int code) {
	if (opt != NULL && opt->state._hts_in_mirror) {
		signal(code, sig_term); // quitter si encore
		printf("\n** Finishing pending transfers.. press again ^C to quit.\n");
		if (opt != NULL) {
			// ask for stop
			opt->state.stop = 1;
		}
	} else {
		sig_term(code);
	}
}

static void signal_handlers(void) {
	signal(SIGHUP, sig_back); // close window
	signal(SIGTSTP, sig_back); // ^Z
	signal(SIGTERM, sig_finish); // kill <process>
#if 0	/* BUG366763 */
	signal( SIGINT , sig_ask ); // ^C
#else
	signal(SIGINT, sig_leave); // ^C
#endif
	signal(SIGPIPE, sig_brpipe); // broken pipe (write into non-opened socket)
	signal(SIGCHLD, sig_ignore); // child change status
	signal(SIGTSTP, SIG_DFL); //^Z
}

static int fexist(const char* s) {
  struct stat st;
  memset(&st, 0, sizeof(st));
  if (stat(s, &st) == 0) {
    if (S_ISREG(st.st_mode)) {
      return 1;
    }
  }
  return 0;
}

static void __cdecl htsshow_init(t_hts_callbackarg *carg) {
}
static void __cdecl htsshow_uninit(t_hts_callbackarg *carg) {
}

static int __cdecl htsshow_start(t_hts_callbackarg *carg, httrackp* opt) {
	return 1;
}

static int __cdecl htsshow_chopt(t_hts_callbackarg *carg, httrackp* opt) {
	return 1;
}
static int __cdecl htsshow_end(t_hts_callbackarg *carg, httrackp* opt) {
	return 1;
}
static int __cdecl htsshow_preprocesshtml(t_hts_callbackarg *carg, httrackp *opt, char** html,int* len,const char* url_address,const char* url_file) {
	return 1;
}
static int __cdecl htsshow_postprocesshtml(t_hts_callbackarg *carg, httrackp *opt, char** html,int* len,const char* url_address,const char* url_file) {
	return 1;
}
static int __cdecl htsshow_checkhtml(t_hts_callbackarg *carg, httrackp *opt, char* html,int len,const char* url_address,const char* url_file) {
	struct hts_proj *proj = CALLBACKARG_USERDEF(carg);
	if (strstr(url_address, "dianying.yisou.com")) {
		fprintf(stderr, "Parsing html file: http://%s%s   [%ldk]\n",url_address, url_file, sizeof(html));
	}
	return 1;
}
static const char* __cdecl htsshow_query(t_hts_callbackarg *carg, httrackp *opt, const char* question) {
	return "N";
}
static const char* __cdecl htsshow_query2(t_hts_callbackarg *carg, httrackp *opt, const char* question) {
	return "N";
}
static const char* __cdecl htsshow_query3(t_hts_callbackarg *carg, httrackp *opt, const char* question) {
	return "";
}
static int __cdecl htsshow_loop(t_hts_callbackarg *carg, httrackp *opt, lien_back* back,int back_max,int back_index,int lien_n,int lien_tot,int stat_time, hts_stat_struct* stats) { // appelé à chaque boucle de HTTrack
	return 1;
}
static int __cdecl htsshow_check(t_hts_callbackarg *carg, httrackp *opt, const char* adr,const char* fil,int status) {
	return -1;
}
static int __cdecl htsshow_check_mime(t_hts_callbackarg *carg, httrackp *opt, const char* adr,const char* fil,const char* mime,int status) {
	return -1;
}
static void __cdecl htsshow_pause(t_hts_callbackarg *carg, httrackp *opt, const char* lockfile) {
	while (fexist(lockfile)) {
		Sleep(1000);
	}
}
static void __cdecl htsshow_filesave(t_hts_callbackarg *carg, httrackp *opt, const char* file) {
}
static void __cdecl htsshow_filesave2(t_hts_callbackarg *carg, httrackp *opt, const char* adr, const char* fil, const char* save, int is_new, int is_modified,int not_updated) {
}
static int __cdecl htsshow_linkdetected(t_hts_callbackarg *carg, httrackp *opt, char* link) {
//	if (strstr(link, "dianying.yisou.com")) {
//		fprintf(stderr, "linkdetected : %s\n", link);
//		return 1;
//	}
	return 1;
}
static int __cdecl htsshow_linkdetected2(t_hts_callbackarg *carg, httrackp *opt, char* link, const char* start_tag) {
	return 1;
}
static int __cdecl htsshow_xfrstatus(t_hts_callbackarg *carg, httrackp *opt, lien_back* back) {
	return 1;
}
static int __cdecl htsshow_savename(t_hts_callbackarg *carg, httrackp *opt, const char* adr_complete,const char* fil_complete,const char* referer_adr,const char* referer_fil,char* save) {
	return 1;
}
static int __cdecl htsshow_sendheader(t_hts_callbackarg *carg, httrackp *opt, char* buff, const char* adr, const char* fil, const char* referer_adr, const char* referer_fil, htsblk* outgoing) {
	return 1;
}
static int __cdecl htsshow_receiveheader(t_hts_callbackarg *carg, httrackp *opt, char* buff, const char* adr, const char* fil, const char* referer_adr, const char* referer_fil, htsblk* incoming) {
	return 1;
}

static void httpd_handler(struct evhttp_request *req, void *arg) {
	struct evbuffer *buf;
	buf = evbuffer_new();

	//get http request url
	char *decode_uri = strdup((char*) evhttp_request_uri(req));
	struct evkeyvalq httpd_query;
	evhttp_parse_query(decode_uri, &httpd_query);
	free(decode_uri);

	//recive http params
	const char *start = evhttp_find_header(&httpd_query, "start");
	const char *httpd_charset = evhttp_find_header(&httpd_query, "charset");

	//send http header charset
	if (httpd_charset != NULL && strlen(httpd_charset) <= 40) {
		char content_type[64] = { 0 };
		sprintf(content_type, "text/plain; charset=%s", httpd_charset);
		evhttp_add_header(req->output_headers, "Content-Type", content_type);
	} else {
		evhttp_add_header(req->output_headers, "Content-Type", "text/plain");
	}

	evhttp_add_header(req->output_headers, "Connection", "keep-alive");
	evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");
	evbuffer_add_printf(buf, "%s\n", "HTTRACK_OK");

	char start_t[255] = {0x00};
	sprintf(start_t, "%s", start);
	if (strcmp(start_t, "1") == 0) {

		struct hts_proj proj;
		proj.depth = 5;
		proj.priority = PRIORITY_ONLY_HTML;
		proj.action = ACTION_ONLY_FILES;

		sprintf(proj.name, "%s", "dianying.yisou.com");
		sprintf(proj.urls, "%s", "http://dianying.yisou.com/");

		new_dl_thread();
	}

	//send content to client
	evhttp_send_reply(req, HTTP_OK, "OK", buf);

	//free buf
	evhttp_clear_headers(&httpd_query);
	evbuffer_free(buf);
}

static void et_init(void) {
	event_init();
	struct httpd_cfg httpd_config;
	httpd_config.listen = "0.0.0.0";
	httpd_config.port = 2688;
	httpd_config.timeout = 60;

	httpd = evhttp_start(httpd_config.listen, httpd_config.port);
	if (httpd == NULL) {
		fprintf(stderr, "\nUnable to listen on %s:%d\n\n", httpd_config.listen,
				httpd_config.port);
		kill(0, SIGTERM);
		exit(1);
	}
	evhttp_set_timeout(httpd, httpd_config.timeout);

	/* Set a callback for all other requests. */
	evhttp_set_gencb(httpd, httpd_handler, NULL);
}

static void et_wait(void) {
	event_dispatch();
}

static void et_uninit() {
	/* Not reached in this code as it is now. */
	evhttp_free(httpd);
}

static void arg_init(struct hts_proj *proj) {
	char * a;
	char cmd[4096] = { 0x00 };
	char str[256] = {0x00};

	proj->argc = 1;
	if (proj->cache == -1) proj->cache = 2; //cache default is best chose
	if (proj->conns == -1) proj->conns = 10; //10 limits the number of connections per second
	if (proj->timeout == -1) proj->timeout = 10;


	if (strnotempty(proj->name) == 0) proj->errorno = 1;
	if (strnotempty(proj->urls) == 0) proj->errorno = 2;

 	while ((a = strchr(proj->urls, ','))) *a = ' ';
	while ((a = strchr(proj->urls, '\t'))) *a = ' ';
	strcat(cmd, proj->urls);
	strcat(cmd, " ");

	//connections limit
	sprintf(str, "-%%c%d ", proj->conns);
	strcat(cmd, str);

	//timeout
	sprintf(str, "-T%d ", proj->timeout);
	strcat(cmd, str);

	if (strnotempty(proj->savepath) == 0) {
		strcat(proj->savepath, getenv("HOME"));
		strcat(proj->savepath, "/websites/");
	}

	if (strnotempty(proj->savepath)){
		if ((proj->savepath[strlen(proj->savepath)-1]!='/') &&
			(proj->savepath[strlen(proj->savepath)-1]!='\\')) {
			 strcat(proj->savepath, "/");
		}
	}

	strcat(cmd, "-q ");

	if (strnotempty(proj->linklist)) {
		strcat(cmd, "-%L");
		strcat(cmd, proj->linklist);
		strcat(cmd, " ");
	}

	sprintf(str, "-C%d ", proj->cache);
	strcat(cmd, str);

	switch (proj->action) {
	case ACTION_ALL_SITES:
		strcat(cmd, "-w ");
		break;
	case ACTION_ONLY_FILES:
		strcat(cmd, "-g ");
		break;
	case ACTION_WITH_CACHE:
		strcat(cmd, "-i ");
		break;
	case ACTION_FIST_LEVEL:
		strcat(cmd, "-Y ");
		break;
	}

	strcat(cmd," --path \"");
	strcat(cmd, proj->savepath);
	strcat(cmd, proj->name);
	strcat(cmd, "\" ");

	sprintf(str, "-p%d ", proj->priority);
	strcat(cmd, str);

	if (strnotempty(proj->proxy_port) == 0) {
		sprintf(proj->proxy_port,"%s", "8080");
	}

	if (proj->retries) {
		sprintf(str, "-R%d ", proj->retries);
		strcat(cmd, str);
	}

	if (strnotempty(proj->proxy)) {
		strcat(cmd, "--proxy ");
		strcat(cmd, proj->proxy);
		strcat(cmd, ":");
		strcat(cmd, proj->proxy_port);
	}

	if (proj->depth > 0) {
		sprintf(str,"-r%d ", proj->depth);
		strcat(cmd, str);
	}

	proj->argv[0] = "httrack";
	proj->argc++;
	proj->argv[1] = cmd;

	fprintf(stderr, "%s\n", cmd);

	int i = 0;
	int g = 0;
	while (cmd[i]) {
		if (cmd[i] == '\"')	g = !g;
		if (cmd[i] == ' ') {
			if (!g) {
				cmd[i] = '\0';
				proj->argv[proj->argc++] = cmd + i + 1;
			}
		}
		i++;
	}
}

void ht_init(void) {
	hts_init();
	opt = hts_create_opt();
};

void start_dl(proj) {

	arg_init(&proj);

	CHAIN_FUNCTION(opt, init, htsshow_init, &proj);
	CHAIN_FUNCTION(opt, uninit, htsshow_uninit, &proj);
	CHAIN_FUNCTION(opt, start, htsshow_start, &proj);
	CHAIN_FUNCTION(opt, end, htsshow_end, &proj);
	CHAIN_FUNCTION(opt, chopt, htsshow_chopt, &proj);
	CHAIN_FUNCTION(opt, preprocess, htsshow_preprocesshtml, &proj);
	CHAIN_FUNCTION(opt, postprocess, htsshow_postprocesshtml, &proj);
	CHAIN_FUNCTION(opt, check_html, htsshow_checkhtml, &proj);
	CHAIN_FUNCTION(opt, query, htsshow_query, &proj);
	CHAIN_FUNCTION(opt, query2, htsshow_query2, &proj);
	CHAIN_FUNCTION(opt, query3, htsshow_query3, &proj);
	CHAIN_FUNCTION(opt, loop, htsshow_loop, &proj);
	CHAIN_FUNCTION(opt, check_link, htsshow_check, &proj);
	CHAIN_FUNCTION(opt, check_mime, htsshow_check_mime, &proj);
	CHAIN_FUNCTION(opt, pause, htsshow_pause, &proj);
	CHAIN_FUNCTION(opt, filesave, htsshow_filesave, &proj);
	CHAIN_FUNCTION(opt, filesave2, htsshow_filesave2, &proj);
	CHAIN_FUNCTION(opt, linkdetected, htsshow_linkdetected, &proj);
	CHAIN_FUNCTION(opt, linkdetected2, htsshow_linkdetected2, &proj);
	CHAIN_FUNCTION(opt, xfrstatus, htsshow_xfrstatus, &proj);
	CHAIN_FUNCTION(opt, savename, htsshow_savename, &proj);
	CHAIN_FUNCTION(opt, sendhead, htsshow_sendheader, &proj);
	CHAIN_FUNCTION(opt, receivehead, htsshow_receiveheader, &proj);

	if (hts_main2(proj.argc, proj.argv, opt)) {
		fprintf(stderr, "hts_main2 with error %s\n", hts_errmsg(opt));
	}
}

void ht_uninit() {
	opt = NULL;
	hts_free_opt(opt);
	hts_uninit();
}

void new_dl_thread(proj) {
	pthread_t tid[100];
	pthread_create(&tid[pid], NULL, (void *) run_task, proj);
//	pthread_join(tid[pid], NULL);
	pid++;
}

int main(int argc, char **argv) {
	httrackp * opt;

	signal_handlers();
	//init hts
	ht_init();
	//init libevent listen port
	et_init();
	//libevent loop
	et_wait();

	ht_uninit();
	//free
	et_uninit();
	return 0;
}
