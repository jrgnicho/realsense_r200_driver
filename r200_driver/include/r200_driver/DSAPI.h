/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014-2015 Intel Corporation. All Rights Reserved.

*******************************************************************************/

#pragma once

// Defines global data types, typedefs, and constants.
#include <r200_driver/DSAPI/DSAPITypes.h>

// Includes definition of calibration struct
#include <r200_driver/DSAPI/DSCalibRectParameters.h>

// Include the other interface classes in this header file as well.
#include <r200_driver/DSAPI/DSHardware.h>
#include <r200_driver/DSAPI/DSFile.h>
#include <r200_driver/DSAPI/DSThird.h>
#include <r200_driver/DSAPI/DSEmitter.h>
#include <r200_driver/DSAPI/DSPlatformCamera.h>

// Include the factory interface.
#include <r200_driver/DSAPI/DSFactory.h>

/// @class DSAPI
/// Defines common interface used in all implementations.
class DS_DECL DSAPI
{
public:
    // Open the DS camera module. This method must be called before any other method.
    /// @param serialNumber When provided, connects with the camera that has this specific serial number, or by default, the first camera detected.
    /// @return true if succeeded or false if failed.
    /// @see closeDevice
    virtual bool openDevice(uint32_t serialNumber = 0xFFFFFFFF) = 0;

    /// Close the DS camera module. This method must called before calling DSDestroy.
    /// @return true if succeeded or false if failed.
    /// @see open
    virtual bool closeDevice() = 0;

    /// This method has to be called right after DSOpen and before any other method. When this method is called we will reach out
    /// to the underlying platform for the first time and probe the device (or files) for a set of relevant properties. If this
    /// method fails, make sure to get the last error details string to see what went wrong.
    /// @return false on fail else true.
    virtual bool probeConfiguration() = 0;

    /// Get the type of the current platform, which is the source of streaming data (see the DSPlatform enum).
    /// @return the type of the current platform
    virtual DSPlatform getPlatform() = 0;

    /// Returns the version number of the DSAPI firmware being used.
    virtual const char * getFirmwareVersionString() = 0;
    virtual void getFirmwareVersionInt(int & v1, int & v2, int & v3, int & v4) = 0;

    /// Returns the version number of the DSAPI software being used.
    virtual const char * getSoftwareVersionString() = 0;

    /// Gets camera serial number from the camera head.
    virtual bool getCameraSerialNumber(uint32_t & number) = 0;

    /// If this DS instance is live hardware, returns an interface that can be used to manipulate hardware settings, otherwise returns nullptr.
    virtual DSHardware * accessHardware() = 0;

    /// If this DS instance has a third imager, returns an interface that can be used to control it, otherwise returns nullptr.
    virtual DSThird * accessThird() = 0;

    /// If this DS instance has an emitter, returns an interface that can be used to control it, otherwise returns nullptr.
    virtual DSEmitter * accessEmitter() = 0;

    /// If this DS instance is being read back from the filesystem, returns an interface that can be used to control playback, otherwise returns nullptr.
    virtual DSFile * accessFile() = 0;

    /// If there in a platform camera (a camera not belonging to a DS device), returns an interface that can be used to control it, otherwise returns nullptr
    /// Must be called after ProbeConfiguration.
    virtual DSPlatformCamera * accessPlatformCamera() = 0;

    /// Get number supported resolution modes (includes pairings with framerates)
    virtual int getLRZNumberOfResolutionModes(bool rectified) = 0;
    /// For each index = 0 to getLRZNumberOfResolutionModes() - 1 will get the z width, z height and framerate.
    /// Will implicitly also set left and right width and height.
    virtual bool getLRZResolutionMode(bool rectified, int index, int & zWidth, int & zHeight, int & lrzFps, DSPixelFormat & lrPixelFormat) = 0;
    /// Set z width, z height and framerate. Notice that by setting the z width and height we also set the left and right width and height.
    virtual bool setLRZResolutionMode(bool rectified, int zWidth, int zHeight, int lrzFps, DSPixelFormat lrPixelFormat) = 0;

    /// Get current frame rate.
    virtual int getLRZFramerate() = 0;

    /// Returns the pixel format of the left and right imager.
    /// @return pixel format of the left and right imager
    virtual DSPixelFormat getLRPixelFormat() = 0;
    /// Returns true if pixel format is native for the left and right imager (in file mode indicates captured data).
    virtual bool isLRPixelFormatNative(DSPixelFormat pixelFormat) = 0;

    /// Returns true if rectification is enabled.
    virtual bool isRectificationEnabled() = 0;

    /// Enables the capture of depth data (default is enabled).
    virtual bool enableZ(bool state) = 0;
    /// Returns true if capture of depth image is enabled.
    virtual bool isZEnabled() = 0;

    /// Enables the capture of left images.
    virtual bool enableLeft(bool state) = 0;
    /// Returns true if capture of left images is enabled.
    virtual bool isLeftEnabled() = 0;

    /// Enables the capture of right images.
    virtual bool enableRight(bool state) = 0;
    /// Returns true if capture of right images is enabled.
    virtual bool isRightEnabled() = 0;

    /// Indicates whether to crop left and right images to match size of z image. If the parameter is true
    /// (the default behavior), the left and right images will be cropped to match the size of the range images.
    virtual bool enableLRCrop(bool state) = 0;
    /// Returns true if cropping is enabled.
    virtual bool isLRCropEnabled() = 0;

    /// Starts the DS system and capturing of frames. Applies all parameters as set at the time of the call. This includes
    /// imager configurations etc. Must be called before grab().
    /// @return false on fail else true.
    virtual bool startCapture() = 0;

    /// Stops the DS system. startCapture and stopCapture can be used in a pair for streaming with differnt configurations.
    /// @return false on fail else true.
    virtual bool stopCapture() = 0;

    /// Grab advances the state to the last set of images captured from the DS System.
    /// After grab, the next calls to image accessors (getZImage, getLImage, getRImage, etc) will return pointers to
    /// the current frame set's image data. Pointers returned by previous calls to image accessors are no longer considered valid.
    /// After grab, the next call to getFrameTime and getFrameNumber will apply to images in the currently valid image set.
    /// If time between grab calls is less than one frame time of the fastest stream enabled, the second grab will block until new data is available.
    /// If time between grab calls is longer than one frame time, the last frame set captured will be available, and older frames will be skipped.
    /// If two streams with different frame rates are enabled, the frame number will advance by one each frame for the fastest
    /// stream. The slower steam's frame number (e.g. third) will not advance if a new synced frame is unavailable,
    /// and the image data will still be the same as the previous frame. e.g. for 60fps z and 30pfs third, the frame
    /// count for third will always advance by two every other grab.
    /// Sync is supported for z/l/r streams and third streams at different frame rates only if: z frame rate > third frame rate, and z frame rate must be a multiple of of third frame rate.
    /// @return false on fail else true.
    virtual bool grab() = 0;

    /// Gets a pointer to the z image data.
    /// Returns a pointer to the frame of z image data captured during the last call to grab(). The data is stored in contiguous rows from top left to
    /// bottom right. The number of rows in the image is given by zHeight() and the number of columns is given by zWidth(). Pixels are 16 bit values.
    /// If capture of z data has not been enabled, results are undetermined. Allocation of this memory is handled internally.
    /// @return pointer to the z image data.
    virtual uint16_t * getZImage() = 0;

    /// Gets a pointer to the left image data.
    /// Returns a pointer to the frame of left image data captured during the last call to grab(). The data is stored in contiguous rows from top left
    /// to bottom left. The number of rows in the image is given by lrHeight() and the number of columns is given by lrWidth().
    /// If any conversions are necessary to provide the user with requested output format, they will be done in this call (not grab).
    /// @return pointer to the left image data.
    virtual void * getLImage() = 0;

    /// Gets a pointer to the right image data.
    /// Returns a pointer to the frame of right image data captured during the last call to grab(). The data is stored in contiguous rows from top left
    /// to bottom right. The number of rows in the image is given by lrHeight() and the number of columns is given by lrWidth().
    /// If any conversions are necessary to provide the user with requested output format, they will be done in this call (not grab).
    /// @return pointer to the right image data.
    virtual void * getRImage() = 0;

    /// Returns width of Z image.
    virtual int zWidth() = 0;
    /// Returns height of Z image.
    virtual int zHeight() = 0;

    /// Returns width of left and right images.
    virtual int lrWidth() = 0;
    /// Returns height of left and right images.
    virtual int lrHeight() = 0;

    /// Returns the number of bits of the maximum value represented in a 16 bit intensity image.
    /// This can be helpful in setting analysis parameters.  Typically the DS system will have either
    /// 10 bit or 12 bit max values in the left and right images.
    virtual int maxLRBits() = 0;

    /// Returns time stamp of the current image data.
    /// If usePerformanceCounter is false, the double is representing time in seconds since 00:00 Coordinated Universal Time (UTC), January 1, 1970.
    /// If usePerformanceCounter is true, the double is representing performance time in seconds
    virtual double getFrameTime(bool usePerformanceCounter = false) = 0;

    /// Gets the current frame number.
    virtual int getFrameNumber() = 0;

    /// Gets the most recently set error status.
    virtual DSStatus getLastErrorStatus() = 0;
    /// Gets the error description string corresponding to the last error.
    virtual const char * getLastErrorDescription() = 0;

    /// Set logging level, including turning logging off, which is the default value.
    /// If logFileName is nullptr then log information is written to a file named "DSAPI.log" created in the current directory.
    virtual void setLoggingLevelAndFile(DSLoggingLevel level, const char * logFileName = nullptr) = 0;
    /// Add a custom message/entry to the log file.
    virtual void logCustomMessage(DSLoggingLevel level, const char * message) = 0;


    /// Returns current disparity shift.
    virtual uint32_t getDisparityShift() = 0;

    /// Switches the range output mode between distance (Z) and disparity (inverse distance).
    /// output = disparity * disparityMultiplier (see set functions below for multiplier)
    virtual bool enableDisparityOutput(bool state) = 0;
    /// Returns true is disparity output is enabled.
    virtual bool isDisparityOutputEnabled() = 0;

    /// Sets the disparity scale factor used when in disparity output mode. Default value is 32.
    virtual bool setDisparityMultiplier(double multiplier) = 0;
    /// Returns the disparity scale factor used when in disparity output mode.
    virtual double getDisparityMultiplier() = 0;

    /// Sets the minimum and maximum z in current z units that will be output.
    /// Any values less than minZ or greater than maxZ will be mapped to 0 during the final conversion between disparity and z. Using a maxZ value of 0xFFFF
    /// corresponds to no thresholding. Use of the conversion utility functions can be valuable to keep code independent of the current setting for z units.
    /// @param minZ The minimum z value that will be output.
    /// @param maxZ The maximum z value that will be output.
    /// @return false on fail else true.
    virtual bool setMinMaxZ(uint16_t minZ, uint16_t maxZ) = 0;
    /// Returns the minimum z and maximum Z in Z units that will be output.
    /// @param minZ The minimum z value that will be output.
    /// @param maxZ The maximum z value that will be output.
    virtual void getMinMaxZ(uint16_t & minZ, uint16_t & maxZ) = 0;

    /// Returns the current units of the Z image in micrometers.
    /// The Z image contains 16 bit pixel values. The units are controllable by system parameters.
    /// This function returns the current units in micrometers.  For example, a value of 1000
    /// indicates that a Z pixel value of 1 should be interpreted as one millimeter.
    /// @return current units of the Z image in micrometers.
    virtual uint32_t getZUnits() = 0;
    /// Sets the current units of the Z image in micrometers.
    /// The Z image contains 16 bit pixel values. The units are controllable by system parameters -
    /// via either this function or an ini file entry. This function sets the current units in micrometers.
    /// For example, a value of 1000 indicates that a Z pixel should be interpreted as (integral) millimeters.
    /// @param units current units of the Z image in micrometers.
    virtual bool setZUnits(uint32_t units) = 0;

    /// Checks to see if camera has valid calibration data
    virtual bool isCalibrationValid() = 0;

    /// Get calibration parameters for currently selected rectified mode
    virtual bool getCalibIntrinsicsZ(DSCalibIntrinsicsRectified & intrinsics) = 0;
    virtual bool getCalibIntrinsicsRectLeftRight(DSCalibIntrinsicsRectified & intrinsics) = 0;
    virtual bool getCalibExtrinsicsRectLeftToRectRight(double & baseline) = 0;

    /// Get calibration parameters for currently selected non-rectified mode
    virtual bool getCalibIntrinsicsNonRectLeft(DSCalibIntrinsicsNonRectified & intrinsics) = 0;
    virtual bool getCalibIntrinsicsNonRectRight(DSCalibIntrinsicsNonRectified & intrinsics) = 0;
    virtual bool getCalibExtrinsicsNonRectLeftToNonRectRight(double rotation[9], double translation[3]) = 0;

    /// When in a rectified mode (otherwise you won't even have Z), ZToDisparityConstant = baseline * focalLengthX,
    /// where baseline is measured in mm, and focalLengthX is measured in pixels.
    /// ZToDisparityConstant is used to convert between Z distance (in mm) and disparity (in pixels), where ZToDisparityConstant / Z = disparity.
    virtual double getZToDisparityConstant() = 0;

    /// Get the transform that maps from Z camera coordinate system to world coordinate system
    /// If you have a 3D point, X, in the Z camera coordiante system, you transform X into
    /// the world coordinate system via X' = rotation * X + translation
    virtual bool getCalibZToWorldTransform(double rotation[9], double translation[3]) = 0;

    /// Gets the entire calibration data struct from camera memory.
    /// @return false on fail else true.
    virtual bool getCalibRectParameters(DSCalibRectParameters & params) = 0;

    /// Sets the path for image files written once recording images is started.
    /// The directory will be created when capture begins if it doesn't exist. Subdirectories
    /// will also be created called Left, Right, Color, Depth. Each frame will be labeled with a number.
    /// @param path Root path for images to be captured.
    virtual void setRecordingFilePath(const char * path) = 0;
    /// Sets the file format that should be used for image files. Default is png.
    virtual void setImageFileFormat(DSImageFileFormat imageFileFormat) = 0;
    /// Start recording data to file.
    /// @param firstImageNumber number to use for first recorded frame (default 1000)
    /// @param everyNthImage record every Nth frame (default is 1, i.e. every frame)
    virtual void startRecordingToFile(bool state, int everyNthImage = 1, bool clearDirectoryBeforeRecording = false) = 0;
    /// Stop recording data to file.
    virtual void stopRecordingToFile() = 0;
    /// Returns true if currently recording data to file.
    virtual bool isRecordingToFileStarted() = 0;
    /// Gets how often images are to be captured. This is the value that is passed in to startRecordingToFile.
    virtual int getCapturingEveryNthImage() = 0;
    /// Record the current frame regardless of whether or not recording has been started or everyNthImage has been satisfied
    virtual void takeSnapshot(bool clearDirectoryBeforeRecording = false) = 0;

    /// Register Callback to supply and manage buffer allocation during capture
    virtual void registerBufferAllocationCallbacks(
        bool (*CreatePool)(int size, void * userContext),
        bool (*GetBuffer)(uint8_t ** out, void * userContext),
        bool (*ReleaseBuffer)(uint8_t * buffer, void * userContext),
        bool (*DestroyPool)(int size, void * userContext), void * userContext) = 0;


protected:
    // Creation (and deletion) of an object of this
    // type is supported through the DSFactory functions.
    DSAPI(){};
    DSAPI(const DSAPI & other) DS_DELETED_FUNCTION;
    DSAPI(DSAPI && other) DS_DELETED_FUNCTION;
    DSAPI & operator=(const DSAPI & other) DS_DELETED_FUNCTION;
    DSAPI & operator=(DSAPI && other) DS_DELETED_FUNCTION;
    virtual ~DSAPI(){};
};
