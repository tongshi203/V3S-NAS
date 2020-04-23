

#include "m88dd3k.h"

void usage(char *exename)
{
    printf("Usage:\n");
    printf("    %s DTMB tuner, frequency\n", exename);
    printf("    led_no = 1, 2, 3 or 4\n");
}

int fp_i2c;

int main(int argc,char **argv)
{

    int adapter_nr = 4; /* probably dynamically determined */
    char filename[20];
    unsigned int fre_MHz;
    if (argc > 3 || argc == 1)
        goto err;

    snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
    fp_i2c = open(filename, O_RDWR);
    if (fp_i2c < 0)
    {
        /* ERROR HANDLING; you can check errno to see what went wrong */
        printf("Can't open /dev/i2c-%d\n",adapter_nr);
        exit(1);
    }

    /** When you have opened the device, you must specify with what device
      address you want to communicate:
      **/
    int addr = M88DD6301_CHIP_ADRRESS; /* The I2C address */

    if (ioctl(fp_i2c, I2C_SLAVE, addr>>1) < 0)
    {
        /* ERROR HANDLING; you can check errno to see what went wrong */
        printf("Can't find DTMB Chip!\n");
        exit(1);
    }

    printf("Open Chip Successfully!\n");

    printf("DTMB initialization...\n");

    Tunner_Demod_Init();

    printf("DTMB initialization successful!\n");

    if(argc == 2)
    {
        if(strlen(argv[1])!=3)
        {
            goto err;
        }
        else
        {
            fre_MHz = (*(argv[1])-0x30)*100 + (*(argv[1]+1)-0x30)*10 + (*(argv[1]+2)-0x30);
            printf("the frequency is %d MHz!\n",fre_MHz);
            if(DM6301_tuner_channel(fre_MHz*1000, MtFeBandwidth_6M) != MtFeErr_Ok)
            {
               printf("Tuner is unlocked! Please Check the tuner!\n");

            }
            else
            {
                 printf("Tuner is locked!\n");
            }
        }

    }
    close(fp_i2c);


	return 0;

err:
    if (fp_i2c > 0)
        close(fp_i2c);
    usage(argv[0]);
    return -1;

}
