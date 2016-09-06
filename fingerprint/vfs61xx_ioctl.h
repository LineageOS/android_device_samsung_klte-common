/*
 * Copyright (C) 2016 The Android Open Source Project
 * Copyright (C) 2016 The Mokee Project
 * Copyright (C) 2016 The CyanogenMod Project
 *
 * Copied from kernel - drivers/fingerprint/vfs61xx.h
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VFS61XX_IOCTL_H_
#define VFS61XX_IOCTL_H_

/* See kernel/include/linux/fprint_secure.h */
#ifndef ENABLE_SENSORS_FPRINT_SECURE
#define ENABLE_SENSORS_FPRINT_SECURE
#endif

#include <linux/ioctl.h>

/* Pass to VFSSPI_IOCTL_REGISTER_DRDY_SIGNAL command */
struct vfsspi_iocRegSignal {
	/* Process ID to which SPI driver sends signal
	 * indicating that DRDY is asserted */
	int userPID;
	int signalID;		/* Signal number */
};

/* Magic number of IOCTL command */
#define VFSSPI_IOCTL_MAGIC    'k'

/*
 * IOCTL commands definitions
 */

#ifndef ENABLE_SENSORS_FPRINT_SECURE
/* Transmit data to the device
and retrieve data from it simultaneously */
#define VFSSPI_IOCTL_RW_SPI_MESSAGE	\
	_IOWR(VFSSPI_IOCTL_MAGIC, 1, unsigned int)
#endif
/* Hard reset the device */
#define VFSSPI_IOCTL_DEVICE_RESET	\
	_IO(VFSSPI_IOCTL_MAGIC,   2)
/* Set the baud rate of SPI master clock */
#define VFSSPI_IOCTL_SET_CLK	\
	_IOW(VFSSPI_IOCTL_MAGIC,  3, unsigned int)
#ifndef ENABLE_SENSORS_FPRINT_SECURE
/* Get level state of DRDY GPIO */
#define VFSSPI_IOCTL_CHECK_DRDY	\
	_IO(VFSSPI_IOCTL_MAGIC,   4)
#endif
/* Register DRDY signal. It is used by SPI driver
 * for indicating host that DRDY signal is asserted. */
#define VFSSPI_IOCTL_REGISTER_DRDY_SIGNAL	\
	_IOW(VFSSPI_IOCTL_MAGIC,  5, unsigned int)
/* Store the user data into the SPI driver. Currently user data is a
 * device info data, which is obtained from announce packet. */
#ifndef ENABLE_SENSORS_FPRINT_SECURE
#define VFSSPI_IOCTL_SET_USER_DATA	\
	_IOW(VFSSPI_IOCTL_MAGIC,  6, unsigned int)
#endif
#ifndef ENABLE_SENSORS_FPRINT_SECURE
/* Retrieve user data from the SPI driver*/
#define VFSSPI_IOCTL_GET_USER_DATA	\
	_IOWR(VFSSPI_IOCTL_MAGIC, 7, unsigned int)
#endif
/* Enable/disable DRDY interrupt handling in the SPI driver */
#define VFSSPI_IOCTL_SET_DRDY_INT	\
	_IOW(VFSSPI_IOCTL_MAGIC,  8, unsigned int)
/* Put device in Low power mode */
#define VFSSPI_IOCTL_DEVICE_SUSPEND	\
	_IO(VFSSPI_IOCTL_MAGIC,	9)
#ifndef ENABLE_SENSORS_FPRINT_SECURE
/* Indicate the fingerprint buffer size for read */
#define VFSSPI_IOCTL_STREAM_READ_START	\
	_IOW(VFSSPI_IOCTL_MAGIC, 10, unsigned int)
/* Indicate that fingerprint acquisition is completed */
#define VFSSPI_IOCTL_STREAM_READ_STOP	\
	_IO(VFSSPI_IOCTL_MAGIC,   11)
/* Retrieve supported SPI baud rate table */
#define VFSSPI_IOCTL_GET_FREQ_TABLE	\
	_IOWR(VFSSPI_IOCTL_MAGIC, 12, unsigned int)
#endif
/* Turn on the power to the sensor */
#define VFSSPI_IOCTL_POWER_ON	\
	_IO(VFSSPI_IOCTL_MAGIC,   13)
/* Turn off the power to the sensor */
#define VFSSPI_IOCTL_POWER_OFF	\
	_IO(VFSSPI_IOCTL_MAGIC,   14)
#ifdef ENABLE_SENSORS_FPRINT_SECURE
/* To disable spi core clock */
#define VFSSPI_IOCTL_DISABLE_SPI_CLOCK	\
	_IO(VFSSPI_IOCTL_MAGIC, 15)
/* To set SPI configurations like gpio, clks */
#define VFSSPI_IOCTL_SET_SPI_CONFIGURATION	\
	_IO(VFSSPI_IOCTL_MAGIC, 16)
/* To reset SPI configurations */
#define VFSSPI_IOCTL_RESET_SPI_CONFIGURATION	\
	_IO(VFSSPI_IOCTL_MAGIC, 17)
#endif
/* get sensor orienation from the SPI driver*/
#define VFSSPI_IOCTL_GET_SENSOR_ORIENT	\
	_IOR(VFSSPI_IOCTL_MAGIC, 18, unsigned int)
#endif /* VFS61XX_IOCTL_H_ */
