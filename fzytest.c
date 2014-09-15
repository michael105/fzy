#include <stdio.h>
#include <string.h>

#include "match.h"
#include "choices.h"

int testsrun = 0, testsfailed = 0, assertionsrun = 0;

#define assert(x) if(++assertionsrun && !(x)){fprintf(stderr, "test \"%s\" failed\n   assert(%s) was false\n   at %s:%i\n\n", __func__, #x, __FILE__ ,__LINE__);return -1;}

void runtest(int (*test)()){
	testsrun++;
	if(test())
		testsfailed++;
}

int test_match(){
	assert(has_match("a", "a"));
	assert(has_match("a", "ab"));
	assert(has_match("a", "ba"));
	assert(has_match("abc", "a|b|c"));

	/* non-match */
	assert(!has_match("a", ""));
	assert(!has_match("a", "b"));
	assert(!has_match("ass", "tags"));

	/* match when query is empty */
	assert(has_match("", ""));
	assert(has_match("", "a"));

	return 0;
}

int test_scoring(){
	/* App/Models/Order is better than App/MOdels/zRder  */
	assert(match("amor", "app/models/order") > match("amor", "app/models/zrder"));

	/* App/MOdels/foo is better than App/M/fOo  */
	assert(match("amo", "app/m/foo") < match("amo", "app/models/foo"));

	/* GEMFIle.Lock < GEMFILe  */
	assert(match("gemfil", "Gemfile.lock") < match("gemfil", "Gemfile"));

	/* GEMFIle.Lock < GEMFILe  */
	assert(match("gemfil", "Gemfile.lock") < match("gemfil", "Gemfile"));

	/* Prefer shorter matches */
	assert(match("abce", "abcdef") > match("abce", "abc de"));

	/* Prefer shorter candidates */
	assert(match("test", "tests") > match("test", "testing"));

	/* Scores first letter highly */
	assert(match("test", "testing") > match("test", "/testing"));

	/* Prefer shorter matches */
	assert(match("abc", "    a b c ") > match("abc", " a  b  c "));
	assert(match("abc", " a b c    ") > match("abc", " a  b  c "));

	return 0;
}

int test_positions_1(){
	size_t positions[3];
	match_positions("amo", "app/models/foo", positions);
	assert(positions[0] == 0);
	assert(positions[1] == 4);
	assert(positions[2] == 5);

	return 0;
}

int test_positions_2(){
	/*
	 * We should prefer matching the 'o' in order, since it's the beginning
	 * of a word.
	 */
	size_t positions[4];
	match_positions("amor", "app/models/order", positions);
	assert(positions[0] == 0);
	assert(positions[1] == 4);
	assert(positions[2] == 11);

	return 0;
}

int test_positions_3(){
	size_t positions[2];
	match_positions("as", "tags", positions);
	assert(positions[0] == 1);
	assert(positions[1] == 3);

	return 0;
}

int test_positions_4(){
	size_t positions[2];
	match_positions("as", "examples.txt", positions);
	assert(positions[0] == 2);
	assert(positions[1] == 7);

	return 0;
}

int test_positions_exact(){
	size_t positions[3];
	match_positions("foo", "foo", positions);
	assert(positions[0] == 0);
	assert(positions[1] == 1);
	assert(positions[2] == 2);

	return 0;
}

int test_choices_empty(){
	choices_t choices;
	choices_init(&choices);
	assert(choices.size == 0);
	assert(choices.available == 0);
	assert(choices.selection == 0);

	choices_prev(&choices);
	assert(choices.selection == 0);

	choices_next(&choices);
	assert(choices.selection == 0);

	choices_free(&choices);
	return 0;
}

int test_choices_1(){
	choices_t choices;
	choices_init(&choices);
	choices_add(&choices, "tags");

	choices_search(&choices, "");
	assert(choices.available == 1);
	assert(choices.selection == 0);

	choices_search(&choices, "t");
	assert(choices.available == 1);
	assert(choices.selection == 0);

	choices_prev(&choices);
	assert(choices.selection == 0);

	choices_next(&choices);
	assert(choices.selection == 0);

	assert(!strcmp(choices_get(&choices, 0), "tags"));
	assert(choices_get(&choices, 1) == NULL);

	choices_free(&choices);
	return 0;
}

void summary(){
	printf("%i tests, %i assertions, %i failures\n", testsrun, assertionsrun, testsfailed);
}

int main(int argc, char *argv[]){
	(void) argc;
	(void) argv;

	runtest(test_match);
	runtest(test_scoring);
	runtest(test_positions_1);
	runtest(test_positions_2);
	runtest(test_positions_3);
	runtest(test_positions_4);
	runtest(test_positions_exact);

	runtest(test_choices_empty);
	runtest(test_choices_1);

	summary();

	/* exit 0 if all tests pass */
	return !!testsfailed;
}