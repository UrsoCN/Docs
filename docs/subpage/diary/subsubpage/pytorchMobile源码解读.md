# pytorchMobile源码解读

PyTorch Android Examples:

> [android-demo-app](https://github.com/pytorch/android-demo-app)

[PYTORCH MOBILE 之ANDROID DEMO源码分析](https://www.freesion.com/article/5678169563/)

[YOLOV5模型](https://github.com/ultralytics/yolov5/releases/tag/v6.1)

[YOLOv5网络详解](https://www.bilibili.com/video/BV1T3411p7zR)

[7.1 MobileNet网络详解](https://www.bilibili.com/video/BV1yE411p7L7)

## 源码分析-HelloWorldApp

1. 向 GRADLE 添加依赖

``` java
    implementation 'androidx.appcompat:appcompat:1.1.0'
    implementation 'org.pytorch:pytorch_android_lite:1.9.0'
    implementation 'org.pytorch:pytorch_android_torchvision:1.9.0'
```

- org.pytorch:pytorch_android ： Pytorch Android API 的主要依赖，包含为4个Android abis (armeabi-v7a, arm64-v8a, x86, x86_64) 的 libtorch 本地库。
- org.pytorch:pytorch_android_torchvision：它是具有将 android.media.image 和 android.graphics.bitmap 转换为 Tensor 的附加库。

要注意需要设置minSdkVersion 21

2. 读取image.jpg

``` java
bitmap = BitmapFactory.decodeStream(getAssets().open("image.jpg"));
```

[Android加载图片的几种方式](https://blog.csdn.net/izzxacbbt/article/details/83628821)

3. 读取模型

``` java
module = LiteModuleLoader.load(assetFilePath(this, "model.pt"));
```

```java
/**
* Copies specified asset to the file in /files app directory and returns this file absolute path.
*
* @return absolute file path
*/
public static String assetFilePath(Context context, String assetName) throws IOException {
    File file = new File(context.getFilesDir(), assetName);
    if (file.exists() && file.length() > 0) {
        return file.getAbsolutePath();
    }
    try (InputStream is = context.getAssets().open(assetName)) {
        try (OutputStream os = new FileOutputStream(file)) {
            byte[] buffer = new byte[4 * 1024];
            int read;
            while ((read = is.read(buffer)) != -1) {
                os.write(buffer, 0, read);
            }
            os.flush();
        }
        return file.getAbsolutePath();
    }
}
```

注意：`LiteModuleLoader.load()`默认是使用CPU处理，之后试试能不能用Vulkan

另外，assetFilePath方法应该可以改进？

``` java
  public static Module load(final String modelPath) {
    return new Module(new LiteNativePeer(modelPath, null, Device.CPU));
  }
```

4. 将图片显示在`ImgaeView`以及将其转换成模型可处理的Tensor

``` java
// showing image on UI
ImageView imageView = findViewById(R.id.image);
imageView.setImageBitmap(bitmap);

// preparing input tensor
final Tensor inputTensor = TensorImageUtils.bitmapToFloat32Tensor(bitmap,
	TensorImageUtils.TORCHVISION_NORM_MEAN_RGB, TensorImageUtils.TORCHVISION_NORM_STD_RGB, MemoryFormat.CHANNELS_LAST);
```

> org.pytorch.torchvision.TensorImageUtils就是org.pytorch:pytorch_android_torchvision库中的一部分，TensorImageUtils.bitmapToFloat32Tensor 创建一个Tensor类型。

> inputTensor 的 大小为 1x3xHxW, 其中 H 和 W 分别为 Bitmap 的高和宽。

5. 运行模型，预测结果

``` java
// running the model
final Tensor outputTensor = module.forward(IValue.from(inputTensor)).toTensor();
// getting tensor content as java array of floats
final float[] scores = outputTensor.getDataAsFloatArray();
// searching for the index with maximum score
float maxScore = -Float.MAX_VALUE;
int maxScoreIdx = -1;
for (int i = 0; i < scores.length; i++) {
    if (scores[i] > maxScore) {
        maxScore = scores[i];
        maxScoreIdx = i;
    }
}

String className = ImageNetClasses.IMAGENET_CLASSES[maxScoreIdx];

// showing className on UI
TextView textView = findViewById(R.id.text);
textView.setText(className);
```

其中：`float maxScore = -Float.MAX_VALUE;`在java中为表示float类型的最小值的常用方法，而反直觉的是：`Float.MAX_VALUE;`表示的是float类型能够表示的最小精度。

TODO: 在相册中选择图片进行预测，可能涉及到图片格式转化(PNG->jpg)

|2022年4月12日|
|---:|

## 源码分析-PytorchDemoApp



|2022年4月12日|
|---:|