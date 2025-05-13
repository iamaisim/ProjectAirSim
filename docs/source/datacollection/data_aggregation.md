# Data Aggregation

Once the data is generated, the data can be aggregated into specific spec formats that can later be used for tasks such as ingestion to data curation tools or fine-tuning of a pre-trained model. The formats we support are:

* [COCO JSON](#coco-json)
* [JSON](#json)
* [JSONL](#jsonl)
* [CSV](#csv)

Below are the structures of these specs for reference:

#### COCO JSON
`Note` - This spec only supports segmentation data (when segmentation data collection is enabled). None of the other specs hold segmentation data
````json
{
    "info": {
        "description": "DFW Takeoff-Landing Data",
        "url": "",
        "version": 1.0,
        "year": 2021,
        "contributor": "Project AirSim"
    },
    "licenses": [
        {
            "id": 0,
            "url": "/",
            "name": "Public Domain"
        }
    ],
    "images": [
        {
            "license": 0,
            "file_name": "images/0.png",
            "coco_url": "{Link to image on AML datastore}", 
            "absolute-url": "{Link to image on blob-container}",
            "height": 225,
            "width": 400,
            "id": "0",
            "date-captured": ""
        }
    ],
    "categories": [
        {
            "supercategory": "none",
            "id": 0,
            "name": "LandingPads"
        }
    ],
    "annotations": [
        {
            "segmentation": [],
            "iscrowd": 0,
            "area": 52200,
            "image_id": "0",
            "bbox": [
                0.21,
                0.0,
                0.58,
                1.0
            ],
            "category_id": 0,
            "id": "0"
        }
    ]
}
````

#### JSON

````json
{
    "baseuri": "{blob-container URL}",
    "categories": [
        "BasicLandingPad"
    ],
    "dataset_name": "DatasetTwoTestTwo",
    "images": [
        {
            "image-width": 400,
            "image-height": 225,
            "file": {
                "hash": "",
                "key": "",
                "object-type": "File",
                "storage-type": "azure-blob",
                "uri": "0.png"
            },
            "format": ".png",
            "split": "validate",
            "polygon": [
               {"3d BBOX Data"}
            ],
            "boxes": [
                {
                    "height": 0.1111111111111111,
                    "left": 0.4375,
                    "score": 1.0,
                    "tag": "AirTaxi",
                    "top": 0.2088888888888889,
                    "width": 0.1225
                }
            ],
            "weather": "RAIN",
            "time": "2022-06-20T07:15:00",
            "geo-location": "DFW",
            "lat-lon": "33.032079,-97.284227"
        }
    ]
}
````
#### JSONL
````json
{"image_url": "{Link to image}", "image-width": 400, "image-height": 225, "label": {"label": "AirTaxi", "isCrowd": false, "label_confidence": [1.0], "topX": 0.4375, "topY": 0.2088888888888889, "bottomX": 0.56, "bottomY": 0.32, "polygon": [[217.0, 72.0], [218.0, 57.0], [182.0, 72.0], [181.0, 57.0], [223.0, 67.0], [224.0, 47.0], [176.0, 67.0], [175.0, 47.0]]}},
````
#### CSV

|ImageName|pose_x |pose_y|pose_z|roll|pitch|yaw        |category|x0 |y0 |x1 |y1 |x2 |y2 |x3 |y3 |x4 |y4 |x5 |y5 |x6 |y6 |x7 |y7 |x_c  |y_c |w  |h  |
|---------|-------|------|------|----|-----|-----------|--------|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|-----|----|---|---|
|0.png    |-557   |5     |-154  |0   |0    |3.141592654|AirTaxi |217|72 |218|57 |182|72 |181|57 |223|67 |224|47 |176|67 |175|47 |199.5|59.5|49 |25 |
|1.png    |-557.04|5     |-154  |0   |0    |3.141592654|AirTaxi |217|72 |218|57 |182|72 |181|57 |223|67 |224|46 |176|67 |175|46 |199.5|59  |49 |26 |