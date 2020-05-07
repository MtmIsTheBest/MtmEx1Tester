#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../Submission/election.h"
#include "../wetPart/elections/mtm_map/map.h"
#include "utils.h"

// Allow malloc to be unstable
#define malloc xmalloc

#define STRESS_INVERTALS_MODIFIER 100
#define MAX_MALLOC_PER_FUNCTION 100

#ifdef __unix__
//#define WITH_FORK
// Fuck Microsoft and all it stands for.
// If you need to debug on this shitty OS, get the errors one by one.
// Also, good luck. You'll need it
#endif

#define SUPER_LONG_STRING                                                      \
    "why how impolite of him i asked him a civil question and he pretended "   \
    "not to hear me thats not at all nice calling after him i say mr white "   \
    "rabbit where are you going hmmm he wont answer me and i do so want to "   \
    "know what he is late for i wonder if i might follow him why not theres "  \
    "no rule that i maynt go where i please ii will follow him wait for me "   \
    "mr white rabbit im coming too falling how curious i never realized that " \
    "rabbit holes were so dark    and so long    and so empty i believe i "    \
    "have been falling for five minutes and i still cant see the bottom hmph " \
    "after such a fall as this i shall think nothing of tumbling downstairs "  \
    "how brave theyll all think me at home why i wouldnt say anything about "  \
    "it even if i fell off the top of the house i wonder how many miles ive "  \
    "fallen by this time i must be getting somewhere near the center of the "  \
    "earth i wonder if i shall fall right through the earth how funny that "   \
    "would be oh i think i see the bottom yes im sure i see the bottom i "     \
    "shall hit the bottom hit it very hard and oh how it will hurt"

#define ASSERT_TEST(expr)                                                 \
    do {                                                                  \
        if (!(expr)) {                                                    \
            printf("\nAssertion failed at %s:%d %s ", __FILE__, __LINE__, \
                   #expr);                                                \
            return false;                                                 \
        }                                                                 \
    } while (0)

#define TEST_WITH_SAMPLE(test, name)         \
    do {                                     \
        Election sample = getSampleData();   \
        printf("Running sub %s ... ", name); \
        if (test(sample)) {                  \
            printf("[OK]\n");                \
        } else {                             \
            printf("[Failed]\n");            \
            g_status = false;                \
        }                                    \
        cleanUp(sample);                     \
    } while (0)

bool isEven(int num) { return num % 2 ? false : true; }

// Problematic with id 0
bool isCorrectArea(int area_id) {
    static int correct_area = -1;
    if (area_id < 0) {
        correct_area = area_id * (-1);
        return true;  // Return value doesn't matter here
    } else {
        assert(correct_area >= 0);
        return area_id == correct_area;
    }
}

AreaConditionFunction specificArea(int area_id) {
    assert(area_id >= 0);
    isCorrectArea((-1) * area_id);
    return isCorrectArea;
}

static bool g_status = true;

Election getSampleData() {
    Election election = electionCreate();
    assert(election != NULL);
    assert(electionAddTribe(election, 11, "tribe a") == ELECTION_SUCCESS);
    assert(electionAddTribe(election, 12, "tribe b") == ELECTION_SUCCESS);
    assert(electionAddTribe(election, 13, "tribe c") == ELECTION_SUCCESS);
    assert(electionAddTribe(election, 14, "tribe d") == ELECTION_SUCCESS);
    assert(electionAddTribe(election, 15, "tribe e") == ELECTION_SUCCESS);
    assert(electionAddArea(election, 21, "area a") == ELECTION_SUCCESS);
    assert(electionAddArea(election, 22, "area b") == ELECTION_SUCCESS);
    assert(electionAddArea(election, 23, "area c") == ELECTION_SUCCESS);
    assert(electionAddArea(election, 24, "area d") == ELECTION_SUCCESS);
    assert(electionAddArea(election, 25, "area e") == ELECTION_SUCCESS);
    assert(electionAddArea(election, 26, "area f") == ELECTION_SUCCESS);
    return election;
}

void cleanUp(Election election) { electionDestroy(election); }

/**
 * The subtests are defined here.
 * Each one targets a specific edge case
 */

bool subAddTribeInvalidId(Election sample) {
    ASSERT_TEST(electionAddTribe(sample, -1, "invalid tribe id") ==
                ELECTION_INVALID_ID);
    // Verify it wasn't added
    ASSERT_TEST(electionGetTribeName(sample, -1) == NULL);
    return true;
}

bool subAddTribeExist(Election sample) {
    // Existing ID
    ASSERT_TEST(electionAddTribe(sample, 11, "already exist") ==
                ELECTION_TRIBE_ALREADY_EXIST);
    // Existing Name
    ASSERT_TEST(electionAddTribe(sample, 1, electionGetTribeName(sample, 11)) ==
                ELECTION_SUCCESS);
    // Make sure names match
    ASSERT_TEST(strcmp(electionGetTribeName(sample, 1),
                       electionGetTribeName(sample, 11)) == 0);
    // Make sure the names are different instances
    ASSERT_TEST(electionGetTribeName(sample, 1) !=
                electionGetTribeName(sample, 11));

    return true;
}

bool subAddTribeLongName(Election sample) {
    ASSERT_TEST(electionAddTribe(sample, 1, SUPER_LONG_STRING) ==
                ELECTION_SUCCESS);
    ASSERT_TEST(electionGetTribeName(sample, 1) != NULL);
    ASSERT_TEST(strcmp(electionGetTribeName(sample, 1), SUPER_LONG_STRING) ==
                0);
    return true;
}

bool subAddTribeInvalidName(Election sample) {
    ASSERT_TEST(electionAddTribe(sample, 1, "UPPER CASE INVALID") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "names.with.dots.invalid") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "hyphens-are-invalid-too") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "underscores_as_well") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "comma,is not valid") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "exclamation!mark") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "question?mark") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "new\nline") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "tab\tname") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "bell\bname") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "Beginning") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddTribe(sample, 1, "endinG") == ELECTION_INVALID_NAME);
    return true;
}

bool subAddTribeValidName(Election sample) {
    ASSERT_TEST(electionAddTribe(sample, 1, "names with spaces") ==
                ELECTION_SUCCESS);
    ASSERT_TEST(electionAddTribe(sample, 2, "nospaces") == ELECTION_SUCCESS);
    ASSERT_TEST(strcmp(electionGetTribeName(sample, 2), "nospaces") == 0);
    // Check empty string
    ASSERT_TEST(electionAddTribe(sample, 3, "") == ELECTION_SUCCESS);
    ASSERT_TEST(strcmp(electionGetTribeName(sample, 3), "") == 0);
    // String with only space
    ASSERT_TEST(electionAddTribe(sample, 4, " ") == ELECTION_SUCCESS);
    ASSERT_TEST(strcmp(electionGetTribeName(sample, 4), " ") == 0);
    return true;
}

// Checking on edge cases integers. Max, min and zero ad id
bool subAddTribeExtremeIdValues(Election sample) {
    ASSERT_TEST(electionAddTribe(sample, INT_MAX, "max int") ==
                ELECTION_SUCCESS);
    ASSERT_TEST(strcmp(electionGetTribeName(sample, INT_MAX), "max int") == 0);

    ASSERT_TEST(electionAddTribe(sample, 0, "zero id") == ELECTION_SUCCESS);
    ASSERT_TEST(strcmp(electionGetTribeName(sample, 0), "zero id") == 0);

    ASSERT_TEST(electionAddTribe(sample, INT_MIN, "min int") ==
                ELECTION_INVALID_ID);
    ASSERT_TEST(electionGetTribeName(sample, INT_MIN) == NULL);
    return true;
}

// Test removing and readding tribe
bool subRemoveTribeReadd(Election sample) {
    ASSERT_TEST(electionRemoveTribe(sample, 11) == ELECTION_SUCCESS);
    ASSERT_TEST(electionAddTribe(sample, 11, "re added") == ELECTION_SUCCESS);
    ASSERT_TEST(strcmp(electionGetTribeName(sample, 11), "re added") == 0);
    ASSERT_TEST(electionRemoveTribe(sample, 11) == ELECTION_SUCCESS);
    ASSERT_TEST(electionAddTribe(sample, 11, "and again") == ELECTION_SUCCESS);
    return true;
}

/**
 * This test makes sure the string sent to tribe as name is copied and not
 * merely the same instance is used
 */
bool subAddTribeVerifyStringsDereferencing(Election sample) {
    char name[] = "some name";
    ASSERT_TEST(electionAddTribe(sample, 1, name) == ELECTION_SUCCESS);
    ASSERT_TEST(strcmp(electionGetTribeName(sample, 1), name) == 0);
    name[0] = 'a';
    ASSERT_TEST(strcmp(electionGetTribeName(sample, 1), name) != 0);
    ASSERT_TEST(electionGetTribeName(sample, 1) != name);

    return true;
}

bool subAddAreaInvalidId(Election sample) {
    ASSERT_TEST(electionAddArea(sample, -1, "invalid area") ==
                ELECTION_INVALID_ID);
    // Verify it wasn't added
    // ASSERT_TEST(electionGetAreaName(sample, -1) == NULL); TODO: What do we
    // check here?
    return true;
}

bool subAddAreaExist(Election sample) {
    // Existing ID
    ASSERT_TEST(electionAddArea(sample, 21, "id exist") ==
                ELECTION_AREA_ALREADY_EXIST);
    // Existing Name
    ASSERT_TEST(electionAddArea(sample, 1, "area a") == ELECTION_SUCCESS);
    // Make sure names match
    // TODO: How can we verify this
    // ASSERT_TEST(strcmp(electionGetAreaName(sample, 1),
    //                    electionGetAreaName(sample, 11)) == 0);
    // Make sure the names are different instances
    // ASSERT_TEST(electionGetAreaName(sample, 1) !=
    //             electionGetAreaName(sample, 11));

    return true;
}

bool subAddAreaLongName(Election sample) {
    ASSERT_TEST(electionAddArea(sample, 1, SUPER_LONG_STRING) ==
                ELECTION_SUCCESS);
    // TODO: What else can we verify here?
    // ASSERT_TEST(electionGetTribeName(sample, 1) != NULL);
    // ASSERT_TEST(strcmp(electionGetTribeName(sample, 1), SUPER_LONG_STRING) ==
    // 0);
    return true;
}

bool subAddAreaInvalidName(Election sample) {
    ASSERT_TEST(electionAddArea(sample, 1, "UPPER CASE INVALID") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "names.with.dots.invalid") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "hyphens-are-invalid-too") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "underscores_as_well") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "comma,is not valid") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "exclamation!mark") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "question?mark") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "new\nline") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "tab\tname") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "bell\bname") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "Beginning") ==
                ELECTION_INVALID_NAME);
    ASSERT_TEST(electionAddArea(sample, 1, "endinG") == ELECTION_INVALID_NAME);
    return true;
}

bool subAddAreaValidName(Election sample) {
    ASSERT_TEST(electionAddArea(sample, 1, "names with spaces") ==
                ELECTION_SUCCESS);
    ASSERT_TEST(electionAddArea(sample, 2, "nospaces") == ELECTION_SUCCESS);
    // TODO: How can we verify
    // ASSERT_TEST(strcmp(electionGetTribeName(sample, 2), "nospaces") == 0);
    // Check empty string
    ASSERT_TEST(electionAddArea(sample, 3, "") == ELECTION_SUCCESS);
    // ASSERT_TEST(strcmp(electionGetTribeName(sample, 3), "") == 0);
    // String with only space
    ASSERT_TEST(electionAddArea(sample, 4, " ") == ELECTION_SUCCESS);
    // ASSERT_TEST(strcmp(electionGetTribeName(sample, 4), " ") == 0);
    return true;
}

// Checking on edge cases integers. Max, min and zero ad id
bool subAddAreaExtremeIdValues(Election sample) {
    ASSERT_TEST(electionAddArea(sample, INT_MAX, "max int") ==
                ELECTION_SUCCESS);
    // TODO: We want to verify it was actually added. Maybe try adding a vote
    // ASSERT_TEST(strcmp(electionGetAreaName(sample, INT_MAX), "max int") ==
    // 0);

    ASSERT_TEST(electionAddArea(sample, 0, "zero id") == ELECTION_SUCCESS);
    // ASSERT_TEST(strcmp(electionGetTribeName(sample, 0), "zero id") == 0);

    ASSERT_TEST(electionAddArea(sample, INT_MIN, "min int") ==
                ELECTION_INVALID_ID);
    // ASSERT_TEST(electionGetAreaName(sample, INT_MIN) == NULL);
    return true;
}

// Test removing and readding area
bool subRemoveAreaReadd(Election sample) {
    ASSERT_TEST(electionRemoveAreas(sample, specificArea(21)) ==
                ELECTION_SUCCESS);
    ASSERT_TEST(electionAddArea(sample, 21, "re added") == ELECTION_SUCCESS);
    ASSERT_TEST(electionRemoveAreas(sample, specificArea(21)) ==
                ELECTION_SUCCESS);
    ASSERT_TEST(electionAddArea(sample, 21, "and again") == ELECTION_SUCCESS);
    return true;
}

bool subStressAddRemoveRepeat(Election sample) {
    bool status = true;
    const int iterations = STRESS_INVERTALS_MODIFIER * 10;

    for (int i = 0; i < iterations; i++) {
        status = status && subRemoveTribeReadd(sample);
    }
    // TODO Add some votes. Can rely on computation test
    for (int i = 0; i < iterations; i++) {
        status = status && subRemoveAreaReadd(sample);
        // TODO: Add some votes
    }

    return status;
}

bool subStressAddThenRemove(Election sample) {
    bool status = true;
    const int iterations = STRESS_INVERTALS_MODIFIER / 20;
    for (int i = 0; i < iterations; i++) {
        ASSERT_TEST(electionAddArea(sample, i + 100, randLowerString(7)) ==
                    ELECTION_SUCCESS);
        ASSERT_TEST(electionAddTribe(sample, i + 100, randLowerString(7)) ==
                    ELECTION_SUCCESS);
    }
    // TODO Add some votes. Can rely on computation test
    for (int i = 0; i < iterations; i++) {
        ASSERT_TEST(electionRemoveAreas(sample, specificArea(i + 100)) ==
                    ELECTION_SUCCESS);
        ASSERT_TEST(electionRemoveTribe(sample, i + 100) == ELECTION_SUCCESS);
        // TODO: Add some votes
    }

    return status;
}

/**
 * Malloc Failures Sub tests
 */
bool subMallErrElectionCreate(Election sample) {
    Election elect = NULL;
    bool has_failed = false;
    xmalloc(-1);
    for (int i = 1;
         i < MAX_MALLOC_PER_FUNCTION && (elect = electionCreate()) == NULL;
         i++) {
        xmalloc(-(i + 1));
        has_failed = true;
    }

    xmalloc(0);
    if (!has_failed) {
        fprintf(stderr, "electionCreate didn't fail even once");
        return false;
    }
    // TODO: Check no tribes no vote no nothing. I think the only way is computeAreasToTribes
    electionDestroy(elect);

    return true;
}

// END SUBTESTS

/**
 * The super tests are defined here.
 * Each includes multiple possible sub tests convering as many edge cases as
 * possible.
 */

void testCreate() {}
void testAddTribe() {
    printf("Testing %s tests:\n", "'Add Tribe'");
    TEST_WITH_SAMPLE(subAddTribeInvalidId, "Invalid Tribe ID");
    TEST_WITH_SAMPLE(subAddTribeLongName, "Long Tribe Name");
    TEST_WITH_SAMPLE(subAddTribeExist, "Pre Existing Tribe/TribeId");
    TEST_WITH_SAMPLE(subAddTribeInvalidName, "Invalid Tribe Names");
    TEST_WITH_SAMPLE(subAddTribeValidName, "Valid Tribe Names");
    TEST_WITH_SAMPLE(subAddTribeVerifyStringsDereferencing,
                     "Dereferencing String Tribe Name");
    TEST_WITH_SAMPLE(subAddTribeExtremeIdValues,
                     "Verify Tribe Extreme Id Values");
}
void testRemoveTribe() {
    printf("Testing %s tests:\n", "'Remove Tribe'");
    TEST_WITH_SAMPLE(subRemoveTribeReadd, "Re-Adding Tribe");
}
void testAddArea() {
    printf("Testing %s tests:\n", "'Add Area'");
    TEST_WITH_SAMPLE(subAddAreaInvalidId, "Invalid Area ID");
    TEST_WITH_SAMPLE(subAddAreaLongName, "Long Area Name");
    TEST_WITH_SAMPLE(subAddAreaExist, "Pre Existing Area/AreaId");
    TEST_WITH_SAMPLE(subAddAreaInvalidName, "Invalid Area Names");
    // TODO: Add this case
    TEST_WITH_SAMPLE(subAddAreaValidName, "Valid Area Names");
    // TEST_WITH_SAMPLE(subAddAreaVerifyStringsDereferencing,
    //                  "Dereferencing String Area Name");
    TEST_WITH_SAMPLE(subAddAreaExtremeIdValues,
                     "Verify Area Extreme Id Values");
}
void testRemoveArea() {
    printf("Testing %s tests:\n", "'Remove Area'");
    TEST_WITH_SAMPLE(subRemoveAreaReadd, "Re-Adding Area");
}
void testRemoveAreas() {}
void testAddVote() {}
void testRemoveVote() {}
void testComputeAreasToTribesMapping() {}
void testSetTribeName() {}
void testGetTribeName() {}
void testMallocFailures() {
    printf("Testing %s tests:\n", "'Malloc Failures Data Integrity'");
    TEST_WITH_SAMPLE(subMallErrElectionCreate, "Create Election");
}
void testDoomsDay() {
    // TODO: Stress Election with lots of adds and removes for both tribes and
    // areas
    printf("Testing %s tests:\n", "'Dooms` Day'");
    TEST_WITH_SAMPLE(subStressAddRemoveRepeat, "Rapid Add and Remove");
    TEST_WITH_SAMPLE(subStressAddThenRemove, "Fill Up Then Clear One By One");
}
void testDestroy() {}

/*The functions for the tests should be added here*/
void (*tests[])(void) = {testCreate,
                         testAddTribe,
                         testRemoveTribe,
                         testAddArea,
                         testRemoveArea,
                         testRemoveAreas,
                         testAddVote,
                         testRemoveVote,
                         testSetTribeName,
                         testGetTribeName,
                         testComputeAreasToTribesMapping,
                         testMallocFailures,
                         testDestroy,
                         testDoomsDay,
                         NULL};

int main(int argc, char* argv[]) {
#ifdef WITH_FORK
    pid_t pid;
    int exit_code;
#endif
    for (int test_idx = 0; tests[test_idx] != NULL; test_idx++) {
#ifdef WITH_FORK
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Forking process failed for test index %d\n",
                    test_idx);
        } else if (pid == 0) {
            // We're in the subprocess
            tests[test_idx]();
            // We don't want to continue the loop after the current test in the
            // subprocess. The main process will do it anyways
            return g_status == true ? 0 : 1;
        } else {
            // We're in the parrent process
            if (waitpid(pid, &exit_code, 0) != pid) {
                exit_code = -1;
            }
            if (exit_code != 0) {
                g_status = false;
            }
        }
#else
        tests[test_idx]();
#endif
    }

    if (g_status) {
        printf("All tests finishes successfully\n");
        return 0;
    } else {
        fprintf(stderr,
                "One or more tests have failed. See above log for more "
                "information.\n");
        return 1;
    }
}