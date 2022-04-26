#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <linux/time.h>

#define S2FS_MAGIC 0x19980122


static struct inode *s2fs_make_inode(struct super_block *sb, int mode){
  struct inode *ret = new_inode(sb);
  kuid_t uid = KUIDT_INIT(0);
  kgid_t gid = KGIDT_INIT(0);

  if(ret){
    ret->i_ino = get_next_ino();
    ret->i_uid = uid;
    ret->i_gid = gid;
    ret->i_blocks = 0;
    ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
    ret->i_mode = mode;
    ret->i_sb = sb;
  }
  return ret;
}

static struct super_operations s2fs_s_ops = {
  .statfs = simple_statfs,
  .drop_inode = generic_delete_inode,
};

static int s2fs_fill_super(struct super_block *sb, void *data, int silent){
  struct inode *root;
  struct dentry *root_dentry;

  sb->s_magic = S2FS_MAGIC;
  sb->s_op = &s2fs_s_ops;

  root = s2fs_make_inode(sb, S_IFDIR | 0755);
  
  if (!root){
    goto out;
  }
  root->i_op = &simple_dir_inode_operations;
  root->i_fop = &simple_dir_operations;

  root_dentry = d_make_root(root);
  
  if(!root_dentry){
    goto out_iput;
  }
  sb->s_root = root_dentry;
  
  return 0;

  out_iput:
  iput(root);
  out:
  return -ENOMEM;
}


static struct dentry *s2fs_get_super(struct file_system_type *fst, int flags, const char *devname, void *data){
  return mount_nodev(fst, flags, data, s2fs_fill_super);
}

static struct file_system_type s2fs_type = {

  .owner = THIS_MODULE,
  .name = "s2fs",
  .mount = s2fs_get_super,
  .kill_sb = kill_litter_super,
};


static int __init s2fs_init(void) {
  return register_filesystem(&s2fs_type);
}

static void __exit s2fs_exit(void) {
  unregister_filesystem(&s2fs_type);
}

MODULE_LICENSE("GPL");
module_init(s2fs_init);
module_exit(s2fs_exit);
