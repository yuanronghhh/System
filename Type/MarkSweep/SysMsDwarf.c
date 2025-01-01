#include <System/Type/MarkSweep/SysMsDwarf.h>
#include <System/Type/MarkSweep/SysMsMap.h>
#include <System/Type/MarkSweep/SysMsBlock.h>

static SysBool dbg_init_from_file(Dwarf_Debug *dbg,
    const SysChar *path,
    Dwarf_Error *pod_err) {
  unsigned groupnumber = DW_GROUPNUMBER_ANY;
  static char true_pathbuf[FILENAME_MAX];
  unsigned tpathlen = FILENAME_MAX;
  Dwarf_Handler errhand = NULL;
  Dwarf_Ptr errarg = NULL;

  return dwarf_init_path(path, true_pathbuf,
      tpathlen, groupnumber, errhand,
      errarg, dbg, pod_err) != DW_DLV_OK;
}

/**
 * sys_dwarf_extact_pointer: extract pointer need to be tracked
 * @path: the execute file with dwarf info
 *
 * Returns: SysMsMap* list
 */
SysHList *sys_ms_dwarf_extact_pointer(const SysChar *path) {

  return NULL;
}
