from collections import OrderedDict
import os

from skimage import transform
import torch
import torch.nn as nn
import torchvision
import torchvision.transforms as transforms


class Resize(object):
    """Resize input image to a given shape"""

    def __init__(self, output_shape):
        assert isinstance(output_shape, (int, tuple))
        if isinstance(output_shape, int):
            self.output_shape = (output_shape, output_shape)
        else:
            self.output_shape = output_shape

    def __call__(self, image):
        new_image = transform.resize(image, self.output_shape)
        return new_image


class ToTensor(object):
    """Convert numpy.ndarrays to Tensors"""

    def __call__(self, image):
        # Numpy image: H x W x C
        # Torch image: C x H x W
        image = image.transpose((2, 0, 1))
        return torch.from_numpy(image).float()


# TODO: Add VAE & CMVAE model runtime classes when needed
class ResNet(nn.Module):
    def __init__(self, configs=None):
        super(ResNet, self).__init__()
        assert configs is not None, "Provide model_configs"
        if configs.get("pretrained", None):
            print("=> using pre-trained model '{}'".format(configs.get("arch", None)))
            self.model = torchvision.models.__dict__[configs.get("arch", None)](
                pretrained=True
            )
        else:
            print("=> CNN head of brain:'{}'".format(configs.get("arch", None)))
            self.model = torchvision.models.__dict__[configs.get("arch", None)]()
        fc_in = self.model.fc.in_features
        pose_dim = 4  # [x, y, z, yaw]
        pose_layer = torch.nn.Sequential(
            OrderedDict([("pose", torch.nn.Linear(fc_in, pose_dim))])
        )
        self.model.fc = pose_layer

    def forward(self, x):
        return self.model(x)


class DronePosePredictor:
    def __init__(self, brain_arch, brain_cnn_arch, trained_brain, gpu):
        self.brain_arch = brain_arch
        self.brain_cnn_arch = brain_cnn_arch
        self.trained_brain = trained_brain
        self.gpu = gpu
        self.model = self.get_model()
        self.brain = self.load_brain()

    def get_model(self):
        model_configs = {
            "brain_arch": self.brain_arch,
            "arch": self.brain_cnn_arch,
            "pretrained": False,
        }
        print(f"=> Brain's model arch: {self.brain_arch}")
        if self.brain_arch == "cnn":
            return ResNet(model_configs)

        elif self.brain_arch == "vae":
            # TODO: Configure using external json or args
            model_configs["fc_hidden1"] = 1024
            model_configs["fc_hidden2"] = 768
            model_configs["drop_p"] = 0.3
            model_configs["latent_dim"] = 256
            return ResNetVAE(model_configs)

        elif self.brain_arch == "cmvae":
            return CMVAE(n_latents=256)
        else:
            print(f"Unknown Brain model arch:{self.brain_arch}")

    def load_brain(self):
        if os.path.isfile(self.trained_brain):
            print("=> loading Brain from checkpoint: '{}'".format(self.trained_brain))
            if self.gpu is None:
                checkpoint = torch.load(self.trained_brain)
                self.model = torch.nn.DataParallel(self.model)
            else:
                # Map model to be loaded to specified single gpu.
                loc = "cuda:{}".format(self.gpu)
                checkpoint = torch.load(self.trained_brain, map_location=loc)
                self.model = torch.nn.DataParallel(self.model, device_ids=[self.gpu])
        else:
            print("=> FATAL: no checkpoint found at '{}'".format(self.trained_brain))
            exit(1)
        self.start_epoch = checkpoint["epoch"]
        best_perf_metric = checkpoint["best_perf_metric"]
        if self.gpu is not None:
            # best_perf_metric may be from a checkpoint from a different GPU
            best_perf_metric = best_perf_metric  # .to(args.gpu)
        self.model.load_state_dict(checkpoint["state_dict"])
        # optimizer.load_state_dict(checkpoint['optimizer'])
        print(
            "=> loaded Brain from checkpoint: '{}' (epoch {})".format(
                self.trained_brain, checkpoint["epoch"]
            )
        )

        return self.model.cuda()

    def predict(self, image_np):
        # compute output based on brain and input image
        # TODO apply normalization if perc brain is on normalized data.
        self.brain.eval()
        input_transformer = transforms.Compose([Resize((224, 224)), ToTensor()])
        image = input_transformer(image_np)
        image = torch.unsqueeze(image, 0)  # 3D -> 4D
        if self.brain_arch == "cnn":
            output = self.model(image)
        elif self.brain_arch == "vae":
            images_recon, z, mu, logvar = self.model(image)
            output = z
        elif self.brain_arch == "cmvae":
            if self.img_loss:
                images_recon, output, mu, logvar = self.model(image)
            else:
                output, mu, logvar = self.model(image)
        output_np = output.cpu().detach().numpy().squeeze()
        # print("Predicted pose:", output_np)
        return output_np
