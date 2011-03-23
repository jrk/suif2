/*-------------------------------------------------------------------
 * profile.cc
 *
 */

#ifdef CURRENTLY_NOT_IMPLEMENTED //@@@

#include "profile.h"
#include <stdlib.h>
#include <string.h>

static char *k_mem_feedback;
static char *k_loop_feedback;
static char *k_for_feedback;
static char *k_if_feedback;

/*--------------------------------------------------------------------
 * profile_table::profile_table
 */
profile_table::profile_table(void)
{
  if_profs = new profile_data_list;
  loop_profs = new profile_data_list;
  runtime_profs = new profile_data_list;
  last_error = NULL;
  runtime_profile_string = NULL;

  /* register annotations */

  k_mem_feedback = lexicon->enter("memory_feedback")->sp;
  if (!lookup_annote(k_mem_feedback)) {
    ANNOTE(k_mem_feedback,       "memory_feedback",          TRUE);
  }
  k_loop_feedback = lexicon->enter("loop_feedback")->sp;
  if (!lookup_annote(k_loop_feedback)) {
    ANNOTE(k_loop_feedback,      "loop_feedback",            TRUE);
  }
  k_for_feedback = lexicon->enter("for_feedback")->sp;
  if (!lookup_annote(k_for_feedback)) {
    ANNOTE(k_for_feedback,       "for_feedback",             TRUE);
  }
  k_if_feedback = lexicon->enter("if_feedback")->sp;
  if (!lookup_annote(k_if_feedback)) {
    ANNOTE(k_if_feedback,        "if_feedback",              TRUE);
  }
}

/*--------------------------------------------------------------------
 * delete_prof_list
 */
static void delete_prof_list(profile_data_list *profs)
{
  profile_data_list_iter iter(profs);
  while (!iter.is_empty()) {
    delete (iter.step());
  }
}

/*--------------------------------------------------------------------
 * profile_table::~profile_table
 *
 */
profile_table::~profile_table(void)
{
  delete_prof_list(if_profs);
  delete (if_profs);
  delete_prof_list(loop_profs);
  delete (loop_profs);
  delete_prof_list(runtime_profs);
  delete (runtime_profs);
}

/*--------------------------------------------------------------------
 * profile_table::load_memprof_data
 *
 */
static boolean has_profile_info;

void profile_table::load_memprof_data(void)
{
  /*
   * scan for profiling info from all fileset entries
   */

  fileset->reset_iter();
  file_set_entry *fse;
  while (fse = fileset->next_file()) {

    fse->reset_proc_iter();
    proc_sym *psym;
    while (psym = fse->next_proc()) {
      boolean was_in_memory = psym->is_in_memory();
      if (!was_in_memory) {
	psym->read_proc();
      }
      tree_proc *proc = psym->block();
      if (!proc) continue;

      has_profile_info = FALSE;
      proc->map((tree_map_f) &scan_memprof_data, this);
#if 0
      if (!was_in_memory && !has_profile_info) {
	psym->flush_proc();
      }
#endif

    }
  }
}

/*--------------------------------------------------------------------
 * profile_table::scan_memprof_data
 *
 */
void profile_table::scan_memprof_data(tree_node *tn, profile_table *prof)
{
  immed_list *imms;

  switch (tn->kind()) {
  case TREE_FOR:
    imms = (immed_list *) tn->peek_annote(k_for_feedback);
    if (imms) {
      loop_branch_profile *p = new loop_branch_profile;
      p->tn = tn;
      p->visit_count = (*imms)[0].integer();
      p->fallthrough_count = (*imms)[1].integer();
      p->avg_trip_count = (*imms)[2].flt();
   
      prof->loop_profs->append(p);
      has_profile_info = TRUE;
    }
    break;

  case TREE_LOOP:
    imms = (immed_list *) tn->peek_annote(k_loop_feedback);
    if (imms) {
      loop_branch_profile *p = new loop_branch_profile;
      p->tn = tn;
      p->visit_count = (*imms)[0].integer();
      p->fallthrough_count = (*imms)[1].integer();
      p->avg_trip_count = (*imms)[2].flt();
   
      prof->loop_profs->append(p);
      has_profile_info = TRUE;
    }
    break;

  case TREE_IF:
    imms = (immed_list *) tn->peek_annote(k_if_feedback);
    if (imms) {
      if_branch_profile *p = new if_branch_profile;
      p->tn = tn;
      p->visit_count = (*imms)[1].integer();
      p->then_count = (*imms)[0].integer();
      p->then_ratio = (*imms)[2].flt();
   
      prof->if_profs->append(p);
      has_profile_info = TRUE;
    }
    break;

  default:
    break;
  }
}

/*--------------------------------------------------------------------
 * profile_table::load_runtime_data
 *
 */
int profile_table::load_runtime_data(char *filename)
{
  FILE *fd = fopen(filename, "r");
  if (!fd) {
    last_error = "Cannot open file";
    return (FALSE);
  }
  if (!parse_runtime_data(fd)) {
    fclose(fd);
    return (FALSE);
  }
  fclose(fd);
  return (TRUE);
}

/*--------------------------------------------------------------------
 * profile_table::parse_runtime_data
 *
 */
void profile_table::bad_data_format(void)
{
  last_error = "Bad data format in profile file";
}

static int skip_past_star_line(FILE *fd)
{
  while (TRUE) {
    char buffer[200];
    char *line = fgets(buffer, 200, fd);
    if (!line) break;

    if (line[0] != ' ')
      continue;
    if (line[1] != '*')
      continue;

    char *ptr;	
    for (ptr = line + 2; *ptr; ptr++) {
      if (*ptr != '*') {
	break;
      }
    }
    if (*ptr == '\n' || *ptr == 0) {
      return (TRUE);
    }
  }
  return (FALSE);
}

int profile_table::parse_runtime_data(FILE *fd)
{
  int num_errors = 0;

  /* summary info */
  char *summary_info = new char[256];
  summary_info[0] = 0;
  size_t summary_info_length = 256;

  if (!skip_past_star_line(fd)) {
    bad_data_format();
    return (FALSE);
  }

  while (TRUE) {
    char buffer[200];
    char *line = fgets(buffer, 200, fd);
    if (!line) {
      bad_data_format();
      return (FALSE);
    }
    if (line[0] == ' ' && line[1] == '*') {
      break;
    }

    if (strlen(summary_info) + strlen(line) >= summary_info_length) {
      summary_info_length *= 2;
      char *tmp = new char[summary_info_length];
      strcpy(tmp, summary_info);
      delete (summary_info);
      summary_info = tmp;
    }
    strcat(summary_info, line);
  }
  runtime_profile_string = summary_info;

  /* proc info */
  proc_runtime_profile *current_prof = NULL;
  while (TRUE) {
    char buffer[200];
    char *the_name, *variable, *value;

    if (!skip_past_star_line(fd)) {
      break;
    }

    char *line = fgets(buffer, 200, fd);
    if (!line) {
      break;
    }
    if (line[0] != ' ' || line[1] !='`') {
      bad_data_format();
      return (FALSE);
    }

    /* parse line "'proc_name' variable = value" */
    char *end_name = strchr(line, '\'');
    if (!end_name) {
      bad_data_format();
      return (FALSE);
    }
    *end_name = 0;
    the_name = &line[2];

    variable = strtok(end_name + 1, "=");
    if (!variable) {
      bad_data_format();
      return (FALSE);
    }
    while (*variable == ' ') variable++;

    value = strtok(NULL, "");
    if (!value) {
      bad_data_format();
      return (FALSE);
    }
    while (*value == ' ') value++;

    /* record data */
    char *proc_name = lexicon->enter(the_name)->sp;
    proc_sym *proc = (proc_sym *) fileset->globals()->
      lookup_sym(proc_name, SYM_PROC);

    if (!current_prof ||
	current_prof->proc_name != proc_name) {

      /* new profile record */
      current_prof = new proc_runtime_profile;
      runtime_profs->append(current_prof);
      current_prof->proc = proc;
      current_prof->proc_name = proc_name;
    }

    if (strncmp(variable, "Time", 4) == 0) {
      if (sscanf(value, "%lf", &current_prof->time) != 1) {
	bad_data_format();
	return (FALSE);
      }
    } else if (strncmp(variable, "Doalls", 6) == 0) {
      if (sscanf(value, "%d", &current_prof->doalls) != 1) {
	bad_data_format();
	return (FALSE);
      }
    } else if (strncmp(variable, "Barriers", 8) == 0) {
      if (sscanf(value, "%d", &current_prof->barriers) != 1) {
	bad_data_format();
	return (FALSE);
      }
    } else if (strncmp(variable, "Sync Neighbors", 14) == 0) {
      if (sscanf(value, "%d", &current_prof->sync_neighbors) != 1) {
	bad_data_format();
	return (FALSE);
      }
    } else if (strncmp(variable, "Locks", 5) == 0) {
      if (sscanf(value, "%d", &current_prof->locks) != 1) {
	bad_data_format();
	return (FALSE);
      }
    } else if (strncmp(variable, "Reductions", 10) == 0) {
      if (sscanf(value, "%d", &current_prof->reductions) != 1) {
	bad_data_format();
	return (FALSE);
      }
    } else {
      num_errors++;
    }

    if (!skip_past_star_line(fd)) {
      bad_data_format();
      return (FALSE);
    }

  }

  if (num_errors) {
    last_error = "Parse errors encountered in runtime data.";
    return (FALSE);
  } else {
    return (TRUE);
  }
}

/*--------------------------------------------------------------------
 * profile_table::sort_helper 
 *
 * Sort the profiles
 */
void profile_table::sort_helper(profile_data_list *plist,
				int (*score_fn)(profile_data *),
				boolean ascending_order)
{
  int total_count = plist->count();
  heap<profile_data *> h(total_count);
  h.set_ascending_order(ascending_order);

  /* add items into the heap */
  profile_data_list_iter iter(plist);
  while (!iter.is_empty()) {
    profile_data *p = iter.step();
    h.add(p, (*score_fn)(p));
  }

  plist->erase();

  /* extract from the heap, and append it back into the list */
  while (!h.is_empty()) {
    profile_data *p = h.remove();
    plist->append(p);
  }
}

#endif
