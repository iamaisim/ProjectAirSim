"""
Copyright (C) Microsoft Corporation. All rights reserved.

Script for defining all the augmentations operations available
Each operation is a subclass of the `DataAugmentation` class
"""

import albumentations as A


class DataAugmentation(object):
    def __init__(self, config: dict):
        self.config = config

    def augment(self, image, bboxes, bbox_param):
        raise NotImplementedError

    def __call__(self, image, bboxes, bbox_params):
        return self.augment(image, bboxes, bbox_params)

    def __repr__(self):
        return self.__class__.__name__

    def __str__(self):
        return self.__class__.__name__


class RandomHorizontalFlip(DataAugmentation):
    def __init__(self, config: dict):
        super().__init__(config)
        self.p = config.get("p", 1)

    def augment(self, image, bboxes, bbox_params):
        transform = A.HorizontalFlip(p=self.p)
        augmented = transform(image=image, bboxes=bboxes, bbox_params=bbox_params)
        return augmented["image"], augmented["bboxes"]

    def __repr__(self):
        return self.__class__.__name__ + f"(p={self.p})"

    def __str__(self):
        return self.__class__.__name__ + f"(p={self.p})"


class RandomVerticalFlip(DataAugmentation):
    def __init__(self, config: dict):
        super().__init__(config)
        self.p = config.get("p", 1)

    def augment(self, image, bboxes, bbox_params):
        transform = A.VerticalFlip(p=self.p)
        augmented = transform(image=image, bboxes=bboxes, bbox_params=bbox_params)
        return augmented["image"], augmented["bboxes"]

    def __repr__(self):
        return self.__class__.__name__ + f"(p={self.p})"

    def __str__(self):
        return self.__class__.__name__ + f"(p={self.p})"


class RandomCrop(DataAugmentation):
    def __init__(self, config: dict):
        super().__init__(config)
        self.height = config.get("crop-height", 224)
        self.width = config.get("crop-width", 224)

    def augment(self, image, bboxes, bbox_params):
        transform = A.RandomCrop(height=self.height, width=self.width)
        augmented = transform(image=image, bboxes=bboxes, bbox_params=bbox_params)
        return augmented["image"], augmented["bboxes"]

    def __repr__(self):
        return self.__class__.__name__ + f"(height={self.height}, width={self.width})"

    def __str__(self):
        return self.__class__.__name__ + f"(height={self.height}, width={self.width})"


class RandomRotate(DataAugmentation):
    def __init__(self, config: dict):
        super().__init__(config)
        self.angular_limit = config.get("angular-limit", 90)
        self.p = config.get("p", 1)

    def augment(self, image, bboxes, bbox_params):
        transform = A.Rotate(
            limit=self.angular_limit, value=None, always_apply=False, p=self.p
        )
        augmented = transform(image=image, bboxes=bboxes, bbox_params=bbox_params)
        return augmented["image"], augmented["bboxes"]

    def __repr__(self):
        return self.__class__.__name__ + f"(limit={self.angular_limit}, p={self.p})"

    def __str__(self):
        return self.__class__.__name__ + f"(limit={self.angular_limit}, p={self.p})"


class RandomAffine(DataAugmentation):
    def __init__(self, config: dict):
        super().__init__(config)
        self.translate_percent = config.get("translate-percent", 10)
        self.scale = config.get("scale", 1)
        self.shear = config.get("shear", 0)
        self.p = config.get("p", 1)

    def augment(self, image, bboxes, bbox_params):
        transform = A.Affine(
            scale=self.scale,
            translate_percent=self.translate_percent,
            shear=self.shear,
            p=self.p,
        )
        augmented = transform(image=image, bboxes=bboxes, bbox_params=bbox_params)
        return augmented["image"], augmented["bboxes"]

    def __repr__(self):
        return (
            self.__class__.__name__
            + f"(translate%={self.translate_percent}, scale={self.scale}, shear={self.shear}, p={self.p})"
        )

    def __str__(self):
        return (
            self.__class__.__name__
            + f"(translate%={self.translate_percent}, scale={self.scale}, shear={self.shear}, p={self.p})"
        )


class RandomBrightnessContrast(DataAugmentation):
    def __init__(self, config: dict):
        super().__init__(config)
        self.brightness_limit = config.get("brightness-limit", 1)
        self.contrast_limit = config.get("contrast-limit", 1)
        self.p = config.get("p", 1)

    def augment(self, image, bboxes, bbox_params):
        transform = A.RandomBrightnessContrast(
            brightness_limit=self.brightness_limit,
            contrast_limit=self.contrast_limit,
            always_apply=False,
            p=self.p,
            brightness_by_max=True,
        )
        augmented = transform(image=image, bboxes=bboxes, bbox_params=bbox_params)
        return augmented["image"], augmented["bboxes"]

    def __repr__(self):
        return (
            self.__class__.__name__
            + f"(brightness limit={self.brightness_limit}, contrast limit={self.contrast_limit}, p={self.p})"
        )

    def __str__(self):
        return (
            self.__class__.__name__
            + f"(brightness limit={self.brightness_limit}, contrast limit={self.contrast_limit}, p={self.p})"
        )


class RandomHueSaturationValue(DataAugmentation):
    def __init__(self, config: dict):
        super().__init__(config)
        self.hue_shift_limit = config.get("hue-shift-limit", 50)
        self.sat_shift_limit = config.get("saturation-shift-limit", 50)
        self.value_shift_limit = config.get("value-shift-limit", 50)
        self.p = config.get("p", 1)

    def augment(self, image, bboxes, bbox_params):
        transform = A.HueSaturationValue(
            hue_shift_limit=self.hue_shift_limit,
            sat_shift_limit=self.sat_shift_limit,
            val_shift_limit=self.value_shift_limit,
            always_apply=False,
            p=self.p,
        )
        augmented = transform(image=image, bboxes=bboxes, bbox_params=bbox_params)
        return augmented["image"], augmented["bboxes"]

    def __repr__(self):
        return (
            self.__class__.__name__
            + f"(hue limit={self.hue_shift_limit},sat limit={self.sat_shift_limit},value limit={self.value_shift_limit}, p={self.p})"
        )

    def __str__(self):
        return (
            self.__class__.__name__
            + f"(hue limit={self.hue_shift_limit},sat limit={self.sat_shift_limit},value limit={self.value_shift_limit}, p={self.p})"
        )


class RandomMotionBlur(DataAugmentation):
    def __init__(self, config: dict):
        super().__init__(config)
        self.blur_limit = config.get("blur-limit", 30)
        self.p = config.get("p", 1)

    def augment(self, image, bboxes, bbox_params):
        transform = A.MotionBlur(
            blur_limit=self.blur_limit,
            always_apply=False,
            p=self.p,
            allow_shifted=True,
        )
        augmented = transform(image=image, bboxes=bboxes, bbox_params=bbox_params)
        return augmented["image"], augmented["bboxes"]

    def __repr__(self):
        return self.__class__.__name__ + f"(blur limit={self.blur_limit}, p={self.p})"

    def __str__(self):
        return self.__class__.__name__ + f"(blur limit={self.blur_limit}, p={self.p})"


class RandomGaussianNoise(DataAugmentation):
    def __init__(self, config: dict):
        super().__init__(config)
        self.var_limit = config.get("variance-limit", 300)
        self.p = config.get("p", 1)

    def augment(self, image, bboxes, bbox_params):
        transform = A.GaussNoise(var_limit=self.var_limit, always_apply=False, p=self.p)
        augmented = transform(image=image, bboxes=bboxes, bbox_params=bbox_params)
        return augmented["image"], augmented["bboxes"]

    def __repr__(self):
        return (
            self.__class__.__name__ + f"(Variance limit={self.var_limit}, p={self.p})"
        )

    def __str__(self):
        return (
            self.__class__.__name__ + f"(Variance limit={self.var_limit}, p={self.p})"
        )
