int CameraClient::getOrientation(int degrees, bool mirror) {
 if (!mirror) {
 if (degrees == 0) return 0;
 else if (degrees == 90) return HAL_TRANSFORM_ROT_90;
 else if (degrees == 180) return HAL_TRANSFORM_ROT_180;
 else if (degrees == 270) return HAL_TRANSFORM_ROT_270;
 } else { // Do mirror (horizontal flip)
 if (degrees == 0) { // FLIP_H and ROT_0
 return HAL_TRANSFORM_FLIP_H;
 } else if (degrees == 90) { // FLIP_H and ROT_90
 return HAL_TRANSFORM_FLIP_H | HAL_TRANSFORM_ROT_90;
 } else if (degrees == 180) { // FLIP_H and ROT_180
 return HAL_TRANSFORM_FLIP_V;
 } else if (degrees == 270) { // FLIP_H and ROT_270
 return HAL_TRANSFORM_FLIP_V | HAL_TRANSFORM_ROT_90;
 }
 }
    ALOGE("Invalid setDisplayOrientation degrees=%d", degrees);
 return -1;
}
