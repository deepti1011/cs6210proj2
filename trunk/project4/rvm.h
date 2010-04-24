struct range {
  int offset;
  int size;
};

typedef struct _segment {
  void* data;
  char* name;
  int size;
  int numRanges;
  struct range* ranges;
} segment;


struct _rvm_t {
  char* dir;
};

typedef struct _rvm_t rvm_t;

typedef struct _trans_t {
  void** segbases;
  int tid;
  int numsegs;
  struct _rvm_t rvm; 
} trans_t;

/*Initialization and Mapping */
rvm_t rvm_init(const char *directory);
void *rvm_map(rvm_t rvm, const char *segname, int size_to_create);
void rvm_unmap(rvm_t rvm, void *segbase);
void rvm_destroy(rvm_t rvm, const char *segname);

/*Transactional Operations*/
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);
void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size);
void rvm_commit_trans(trans_t tid);
void rvm_abort_trans(trans_t tid);

/*Log Control Operations*/
void rvm_truncate_log(rvm_t rvm);

/*Library Output*/
void rvm_verbose(int enable_flag);

/*Helper methods*/
void write_lock(int fd);
void unlock(int fd);
struct flock* file_lock(short type, short whence);
