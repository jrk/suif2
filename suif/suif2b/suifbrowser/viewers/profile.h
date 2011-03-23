/*--------------------------------------------------------------------
 * profile.h
 *
 */
#ifdef CURRENTLY_NOT_IMPLEMENTED //@@@

#ifndef PROFILE_H
#define PROFILE_H

#include "includes.h"

/*
 * profiling information
 */

enum profile_kind {
  PROF_LOOP_BRANCH,
  PROF_IF_BRANCH,
  PROF_PROC_RUNTIME
};

struct profile_data {
  int kind;
};


/*
 * Data collected from memprof passes
 */

struct loop_branch_profile : public profile_data {
  tree_node *tn;
  int visit_count;
  int fallthrough_count;
  double avg_trip_count;
};

struct if_branch_profile : public profile_data {
  tree_node *tn;
  int visit_count;
  int then_count;
  double then_ratio;
};

/*
 * Data collected from runtime library
 */
struct proc_runtime_profile : public profile_data {
  char *proc_name;
  proc_sym *proc;

  double time;
  int doalls;
  int barriers;
  int sync_neighbors;
  int locks;
  int reductions;

  proc_runtime_profile() {
    time = 0;
    doalls = barriers = sync_neighbors = locks = reductions = 0;
  }
};

DECLARE_LIST_CLASS(profile_data_list, profile_data *);

/*
 * profile_table
 */
class profile_table {

 private:
  int parse_runtime_data( FILE* fd );
  void bad_data_format();
  static void scan_memprof_data( tree_node* tn, profile_table *prof);

 public:
  profile_table();
  ~profile_table();

  char *last_error;
  char *runtime_profile_string;

  profile_data_list *if_profs;
  profile_data_list *loop_profs;
  profile_data_list *runtime_profs;

  void load_memprof_data();
  int load_runtime_data( char* filename );

  void sort_helper( profile_data_list* plist,
		    int (*score_fn)(profile_data *),
		    boolean ascending_order );

};

#endif

#endif












