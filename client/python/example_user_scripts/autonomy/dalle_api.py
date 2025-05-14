"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
ProjectAirSim:: Autonomy building-blocks:: DALL-E API
"""
import os
import requests
from PIL import Image
import openai


class DalleImageGenerator:
    def __init__(self):
        if not os.environ.get("OPENAI_API_KEY"):
            print("Please set the OPENAI_API_KEY environment variable to use the DALL-E API.")
            return None

    @classmethod
    def generate(cls, prompt: str, num: int, size: str, output_dir: str):
        try:
            # Image Generation using OpenAI API
            response = openai.Image.create(prompt=prompt, n=num, size=size)
            return cls._save_image(response, num, output_dir)

        except openai.error.OpenAIError as e:
            # ToDO: Handle OpenAI API errors here, e.g. retry or log
            print(f"OpenAI API error occurred: {e}")
            return None

    @classmethod
    def edit(
        cls,
        prompt: str,
        image_path: str,
        mask_path: str,
        num: int,
        size: str,
        output_dir: str,
    ):
        try:
            # Image Editing using OpenAI API
            image1 = Image.open(image_path).convert("RGBA")
            image2 = Image.open(mask_path).convert("RGBA")

            # Save the converted image to a temporary file
            temp_file1 = "./images/image_temp.png"
            image1.save(temp_file1)

            temp_file2 = "./images/mask_temp.png"
            image2.save(temp_file2)

            response = openai.Image.create_edit(
                image=open(temp_file1, "rb"),
                mask=open(temp_file2, "rb"),
                prompt=prompt,
                n=num,
                size=size,
            )
            return cls._save_image(response, num, output_dir)

        except (IOError, openai.error.OpenAIError) as e:
            # ToDO: Handle errors here, e.g. retry or log
            print(f"Error occurred while processing image edit: {e}")
            return None

    @staticmethod
    def _save_image(response, num, output_dir):
        images_urls = []
        for i in range(num):
            image_url = response["data"][i]["url"]
            images_urls.append(image_url)
            image_data = requests.get(image_url).content
            # Creating a folder to save the image (if it doesn't exist)
            folder_name = output_dir
            if not os.path.exists(folder_name):
                os.makedirs(folder_name)
            # Saving the image
            img_name = f"./{folder_name}{i}.png"
            with open(img_name, "wb") as handler:
                handler.write(image_data)
        print(f"Images saved in {os.path.abspath(folder_name)}")
        return images_urls
