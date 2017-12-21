#ifndef __KERNEL__
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif

#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

#define CTRL_VAL_REGAX 10
#define CTRL_VAL_REGDX 11
#define CTRL_VAL_ABUF 12
#define CTRL_VAL_BFDEL 13
#define CTRL_VAL_ABDEL 14
#define CTRL_FUNC_AROR 15
#define CTRL_FUNC_AROL 16
#define CTRL_FUNC_CHGB 17
#define CTRL_FUNC_XOR 20
#define CTRL_FUNC_ROR 21
#define CTRL_FUNC_ROL 22
#define CTRL_FUNC_SHL 23
#define CTRL_FUNC_SHR 24
#define CTRL_FUNC_SUB 25
#define CTRL_FUNC_ADD 26
#define CTRL_FUNC_EXP 27
#define CTRL_CHAR_ROR 31
#define CTRL_CHAR_ROL 32
#define CTRL_CHAR_HALF 33
#define CTRL_CHAR_MERGE 34
#define CTRL_CHAR_INPUT 35
#define CTRL_CHAR_OUTPUT 36
#define CTRL_CHAR_INABF 37
#define CTRL_CHAR_OUTABF 38
#define CTRL_INTER_XOR 39
#define CTRL_INTER_CMOV 40
//#define CTRL_CTX_CODE 40 //커스텀어셈블리를 통해 레지스터 연산
//#define CTRL_CTX_ONEB 41 //1Byte 연산(한글자)
#define CTRL_CTX_ANTI 99

static DEFINE_MUTEX(mutexIoctl);

static unsigned char* chpBuf = NULL;
static unsigned char* chpABuf = NULL;

static unsigned long ulRegAX = 0; //Offset
static unsigned long ulRegDX = 0; //Size

static int memOpen(struct inode *inp, struct file *fp);
static int memRel(struct inode *inp, struct file *fp);
static ssize_t memRead(struct file *fp, unsigned char *chpOut, size_t sztCount, loff_t *loftPos);
static ssize_t memWrite(struct file *fp, const unsigned char __user *chpIn, size_t sztCount, loff_t *loftPos);
static long memIoctl(struct file *fp, unsigned int iNum, unsigned long iParam);
static void xorFunc(unsigned char *_chpIn, const unsigned char __user *_ulParam, int *_iIter);
static void rorNtFunc(unsigned char *_chpIn, unsigned long _lIdx, unsigned long _lNum);
static void rolNtFunc(unsigned char *_chpIn, unsigned long _lIdx, unsigned long _lNum);
static void shlNtFunc(unsigned char *_chpIn, unsigned long _lIdx, unsigned long _lNum);
static void shrNtFunc(unsigned char *_chpIn, unsigned long _lIdx, unsigned long _lNum);
static void subFunc(unsigned char *_chpIn, const unsigned char __user *_ulParam, int *_iIter);
static void addFunc(unsigned char *_chpIn, const unsigned char __user *_ulParam, int *_iIter);
static void expFunc(unsigned long _lNum, int *_iIter);

static struct file_operations fopsRevme;
//static struct cdev cdevDevice;


//static dev_t devtNum;
static bool bFlag = false;

static int __init dimicraft_init(void)
{
    void *vpErr;
    fopsRevme.read = memRead;
    fopsRevme.write = memWrite;
    fopsRevme.open = memOpen;
    fopsRevme.release = memRel;
    fopsRevme.unlocked_ioctl = memIoctl;

//    if(alloc_chrdev_region(&devtNum, 0, 1, "korev") < 0)
//        return -EIO;

//    clspClass = class_create(THIS_MODULE, "korev");
//    if(IS_ERR(vpErr = clspClass)){
//        unregister_chrdev_region(&devtNum, 1);
//        return -EIO;
//    }

//    devpDevice = device_create(clspClass, NULL, &devtNum, NULL, "korev");
//    if(IS_ERR(vpErr = devpDevice)){
//        class_destroy(clspClass);
//        unregister_chrdev_region(&devtNum, 1);
//        return -EIO;
//    }

    if(register_chrdev(222, "dimicraft", &fopsRevme) < 0)
        return -EIO;

    chpBuf = kmalloc(32, GFP_USER);
    if(IS_ERR(vpErr = chpBuf))
        return -ENOMEM;
    memset(chpBuf, 0, 32);

    chpABuf = kmalloc(32, GFP_USER);
    if(IS_ERR(vpErr = chpABuf))
        return -ENOMEM;
    memset(chpABuf, 0, 32);
    return 0;
}

static void __exit dimicraft_exit(void)
{
       unregister_chrdev(222, "dimicraft");
       if(chpBuf)
        kfree(chpBuf);
       if(chpABuf)
        kfree(chpABuf);
}

static int memOpen(struct inode *inp, struct file *fp){
    if(bFlag)
        return -EBUSY;
    bFlag = true;
    return 0;
}

static int memRel(struct inode *inp, struct file *fp){
    bFlag = false;
    return 0;
}

static ssize_t memRead(struct file *fp, unsigned char __user *chpOut, size_t sztCount, loff_t *loftPos){
    if(sztCount > 32)
        return -1;
    if(bFlag)
        copy_to_user(chpOut, chpBuf, sztCount);
    return sztCount;
}

static ssize_t memWrite(struct file *fp, const unsigned char __user *chpIn, size_t sztCount, loff_t *loftPos){
    if(sztCount > 32)
        return -1;
    if(bFlag)
        copy_from_user(chpBuf, chpIn, sztCount);
    return sztCount;
}

static long memIoctl(struct file *fp, unsigned int uiNum, unsigned long ulParam)
{
    int iIter = 0;
    mutex_lock(&mutexIoctl);
    switch (uiNum)
    {
        case CTRL_VAL_REGAX:
            if(ulParam > 31 || ulParam < 0){
                mutex_unlock(&mutexIoctl);
                return -1;
            }
            ulRegAX = ulParam;
            break;

        case CTRL_VAL_REGDX:
//            if(ulParam > 32){
//                mutex_unlock(&mutexIoctl);
//                return -1;
//            }
            ulRegDX = ulParam;
            break;

        case CTRL_VAL_ABUF:
            if(bFlag){
                if(ulParam > 32){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                copy_from_user(chpABuf, (const unsigned char __user *)ulParam, ulRegDX);
                break;
            }
            else
                break;

        case CTRL_VAL_BFDEL:
            memset(chpBuf, 0, 32);
            break;

        case CTRL_VAL_ABDEL:
            memset(chpABuf, 0, 32);
            break;

        case CTRL_FUNC_AROR:
                if(bFlag){
                    if(ulParam > 31){
                        mutex_unlock(&mutexIoctl);
                        return -1;
                    }
                    rorNtFunc(chpABuf, ulParam, ulRegDX);
                    break;
                }
                else
                    break;

        case CTRL_FUNC_AROL:
            if(bFlag){
                if(ulParam > 31){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                rolNtFunc(chpABuf, ulParam, ulRegDX);
                break;
            }
            else
                break;

        case CTRL_FUNC_CHGB:
            {
                unsigned char *chpTmp;
                chpTmp = chpBuf;
                chpBuf = chpABuf;
                chpABuf = chpTmp;
                break;
             }

        case CTRL_FUNC_XOR:
            if(bFlag){
                unsigned char *chpTmp;
                if(ulRegDX > 31 || ulRegDX < 1 || ulRegAX < 0 || ulRegAX > 30){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                if((chpTmp = kmalloc(32, GFP_USER)) == 0){
                    mutex_unlock(&mutexIoctl);
                    return -ENOMEM;
                }
                memset(chpTmp, 0, 32);
                xorFunc(chpTmp, (const unsigned char __user *)ulParam, &iIter);
                kfree(chpTmp);
                break;
            }
            else
                break;

        case CTRL_FUNC_ROR:
                if(bFlag){
                    if(ulParam > 31){
                        mutex_unlock(&mutexIoctl);
                        return -1;
                    }
                    rorNtFunc(chpBuf, ulParam, ulRegDX);
                    break;
                }
                else
                    break;

        case CTRL_FUNC_ROL:
            if(bFlag){
                if(ulParam > 31){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                rolNtFunc(chpBuf, ulParam, ulRegDX);
                break;
            }
            else
                break;

        case CTRL_FUNC_SHL:
            if(bFlag){
                if(ulParam > 31){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                shlNtFunc(chpBuf, ulParam, ulRegDX);
                break;
            }
            else
                break;

        case CTRL_FUNC_SHR:
            if(bFlag){
                if(ulParam > 31){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                shrNtFunc(chpBuf, ulParam, ulRegDX);
                break;
            }
            else
                break;

        case CTRL_FUNC_SUB:
            if(bFlag){

                unsigned char *chpTmp;
                if(ulRegDX > 31 || ulRegDX < 1 || ulRegAX < 0 || ulRegAX > 30){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                if((chpTmp = kmalloc(32, GFP_USER)) == 0){
                    mutex_unlock(&mutexIoctl);
                    return -ENOMEM;
                }
                memset(chpTmp, 0, 32);
                subFunc(chpTmp, (const unsigned char __user *)ulParam, &iIter);
                kfree(chpTmp);
                break;
            }
            else
                break;

        case CTRL_FUNC_ADD:
            if(bFlag){
                unsigned char *chpTmp;
                if(ulRegDX > 31 || ulRegDX < 1 || ulRegAX < 0 || ulRegAX > 30){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                if((chpTmp = kmalloc(32, GFP_USER)) == 0){
                    mutex_unlock(&mutexIoctl);
                    return -ENOMEM;
                }
                memset(chpTmp, 0, 32);
                addFunc(chpTmp, (const unsigned char __user *)ulParam, &iIter);
                kfree(chpTmp);
                break;
            }
            else
                break;

        case CTRL_FUNC_EXP:
            if(bFlag){
                if(ulRegDX > 31 || ulRegDX < 1 || ulRegAX < 0 || ulRegAX > 30){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                expFunc(ulParam, &iIter);
                break;
            }
            else
                break;


        case CTRL_CHAR_ROR:
            if(bFlag){
                int iTmpIter = 0;
                unsigned char chTmp;
                if(ulParam > 31+1)
                    ulParam %= 31;
                for(iIter = 0; iIter < ulParam; ++iIter){
                    for(iTmpIter = 31; iTmpIter >= 0; --iTmpIter){
                        if(iTmpIter == 0)
                            chpBuf[0] = chTmp;
                        else if(iTmpIter == 31)
                            chTmp = chpBuf[30];
                        else
                            chpBuf[iTmpIter] = chpBuf[iTmpIter-1];
                    }
                }
                iIter = 0;
                break;
            }
            else
                break;


        case CTRL_CHAR_ROL:
            //printk(KERN_INFO "chpBuf : %s\nchpBuf[%ld] : 0x%x\n", chpBuf, 31, chpBuf[31]);
            if(bFlag){
                int iTmpIter = 0;
                unsigned char chTmp;
                if(ulParam > 31+1)
                    ulParam %= 31;
                for(iIter = 0; iIter < ulParam; ++iIter){
                    for(iTmpIter = 0; iTmpIter <= 31; ++iTmpIter){
                        if(iTmpIter == 0)
                            chTmp = chpBuf[0];
                        else if(iTmpIter == 31)
                            chpBuf[30] = chTmp;
                        else
                            chpBuf[iTmpIter-1] = chpBuf[iTmpIter];
                    }
                }
                iIter = 0;
                break;
            }
            else
                break;


        case CTRL_CHAR_HALF:
            for(iIter =15; iIter < 31; ++iIter){
                chpABuf[iIter] = chpBuf[iIter];
            }
            iIter = 0;

        case CTRL_CHAR_MERGE:
            for(iIter = 15; iIter < 31; ++iIter){
                chpBuf[iIter] = chpABuf[iIter];
            }
            iIter = 0;
            break;

        case CTRL_CHAR_INPUT:
            if(bFlag){
                if(ulRegAX > 30 || ulRegAX < 0){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }mutex_unlock(&mutexIoctl);
                copy_from_user(&chpBuf[ulRegAX], (const unsigned char __user *)ulParam, 1);
                break;
            }
            else
                break;

        case CTRL_CHAR_OUTPUT:
            if(bFlag){
                if(ulRegAX > 30 || ulRegAX < 0){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                copy_to_user((unsigned char __user *)ulParam, &chpBuf[ulRegAX], 1);
                break;
            }
            else
                break;

        case CTRL_CHAR_INABF:
            if(bFlag){
                if(ulRegAX > 30 || ulRegAX < 0){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                copy_from_user(&chpABuf[ulRegAX], (const unsigned char __user *)ulParam, 1);
                break;
            }
            else
                break;

        case CTRL_CHAR_OUTABF:
            if(bFlag){
                if(ulRegAX > 30 || ulRegAX < 0){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                copy_to_user((unsigned char __user *)ulParam, &chpABuf[ulRegAX], 1);
                break;
            }
            else
                break;

        case CTRL_INTER_XOR:
                if(bFlag){
                    if(ulParam > 31 || ulRegAX < 0){
                        mutex_unlock(&mutexIoctl);
                        return -1;
                    }

                    for(iIter = ulRegAX; iIter < ulParam; ++iIter){
                        chpBuf[iIter] ^= chpABuf[iIter];
                    }
                    iIter = 0;
                    break;
                }
                else
                    break;

          case CTRL_INTER_CMOV:
                if(ulRegDX > 30 || ulRegDX < 0 || ulRegAX < 0 || ulRegAX > 30){
                    mutex_unlock(&mutexIoctl);
                    return -1;
                }
                if(ulParam == 0)
                    chpABuf[ulRegDX] = chpABuf[ulRegAX];
                if(ulParam == 1)
                    chpBuf[ulRegDX] = chpBuf[ulRegAX];
                if(ulParam == 2)
                    chpABuf[ulRegDX] = chpBuf[ulRegAX];
                else
                    chpBuf[ulRegDX] = chpABuf[ulRegAX];
                break;


//        case CTRL_CTX_CODE:
//        break;

//        case CTRL_CTX_ONEB:
//            switch(ulParam){
//                case CTRL_FUNC_XOR:
//                    if(bFlag){
//                        unsigned char *chpTmp;
//                        printk(KERN_INFO "BreakPoint00\n");
//                        if(ulRegAX < 0 || ulRegAX > 30){
//                            mutex_unlock(&mutexIoctl);
//                            return -1;
//                        }
//                        printk(KERN_INFO "BreakPoint01\n");
//                        if((chpTmp = kmalloc(32, GFP_USER)) == 0){
//                            mutex_unlock(&mutexIoctl);
//                            return -ENOMEM;
//                        }
//                        printk(KERN_INFO "BreakPoint02\n");
//                        memset(chpTmp, 0, 32);
//                        printk(KERN_INFO "BreakPoint03\n");
//                        copy_from_user(chpTmp, (const unsigned char __user *)ulRegDX, 1);
//                        printk(KERN_INFO "BreakPoint04\nKernel Pointer : 0x%lx\nUser Pointer : 0x%lx\n", (unsigned long)chpTmp, ulRegDX);
//                        printk(KERN_INFO "BreakPoint05\nCharacter : %c\n", ((const unsigned char __user *)ulRegDX)[0]);
//                        chpBuf[ulRegAX] ^= chpTmp[0];
//                        kfree(chpTmp);
//                        break;
//                    }
//                    else
//                        break;

//                case CTRL_FUNC_ADD:
//                case CTRL_FUNC_SUB:
//                default:
//                    break;
//            }

        case CTRL_CTX_ANTI:
            __asm__ __volatile__("addq $0x100, %%rsp\n\t"::);
            break;

        default:
            break;

    }
    mutex_unlock(&mutexIoctl);
    return 0;
}

static void xorFunc(unsigned char *_chpIn, const unsigned char __user *_ulParam, int *_iIter){
    copy_from_user(_chpIn, _ulParam, ulRegDX);
    for(*_iIter = ulRegAX; *_iIter < ulRegDX; ++*_iIter){
        chpBuf[*_iIter] ^= _chpIn[*_iIter];
    }
    *_iIter = 0;
}
static void rorNtFunc(unsigned char *_chpIn, unsigned long _lIdx, unsigned long _lNum){
    unsigned char chTmp = _chpIn[_lIdx];
    if(_lNum > 7)
        _lNum %= 8;
    _chpIn[_lIdx] = ((chTmp << _lNum) | (chTmp >> (8 - _lNum)));
}
static void rolNtFunc(unsigned char *_chpIn, unsigned long _lIdx, unsigned long _lNum){
    unsigned char chTmp = _chpIn[_lIdx];
    if(_lNum > 7)
        _lNum %= 8;
    _chpIn[_lIdx] = ((chTmp >> _lNum) | (chTmp << (8 - _lNum)));
}
static void shlNtFunc(unsigned char *_chpIn, unsigned long _lIdx, unsigned long _lNum){
    unsigned char chTmp = _chpIn[_lIdx];
    if(_lNum > 7)
        _lNum %= 8;
    _chpIn[_lIdx] = chTmp << _lNum;
}
static void shrNtFunc(unsigned char *_chpIn, unsigned long _lIdx, unsigned long _lNum){
 unsigned char chTmp = _chpIn[_lIdx];
 if(_lNum > 7)
     _lNum %= 8;
 _chpIn[_lIdx] = chTmp >> _lNum;
}
static void subFunc(unsigned char *_chpIn, const unsigned char __user *_ulParam, int *_iIter){
    copy_from_user(_chpIn, _ulParam, ulRegDX);
    for(*_iIter = ulRegAX; *_iIter < ulRegDX; ++*_iIter){
        chpBuf[*_iIter] -= _chpIn[*_iIter];
    }
    *_iIter = 0;
}
static void addFunc(unsigned char *_chpIn, const unsigned char __user *_ulParam, int *_iIter){
    copy_from_user(_chpIn, _ulParam, ulRegDX);
    for(*_iIter = ulRegAX; *_iIter < ulRegDX; ++*_iIter){
        chpBuf[*_iIter] -= _chpIn[*_iIter];
    }
    *_iIter = 0;
}
static void expFunc(unsigned long _lNum, int *_iIter){
    int iTmpIter = 0;
    for(*_iIter = ulRegAX; *_iIter < ulRegDX; ++*_iIter){
        for(iTmpIter = 0; iTmpIter < _lNum; ++iTmpIter){
            chpBuf[*_iIter] *= chpBuf[*_iIter];
        }
    }
    *_iIter = 0;
}


module_init(dimicraft_init);
module_exit(dimicraft_exit);
