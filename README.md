# VisionComputing-KMeasn

This is the repository for K-Means alogirhm in Vision Computing course

Author: Haoxiang Zhang

# Assignment Requirements

## Assignment 4: Interactive K-Means Clustering Using Qt

Implement the K-Means clustering algorithm presented in class with interactive visualization of the clustering process in **both 2D and 3D**. The dimensionality of the datasets (ie feature points) can be D-dimensional but the visualization of the data will be 2D or 3D. You can assume that each observation or feature point has a unique membership label (ie each point belongs to one and only one class) so you don't need to support fuzzy membership of the same point in multiple clusters. For equidistant points break ties randomly (that is a point is equally distant to several cluster centers). Visualize the evolution of the class membership of each point using colors for each iteration of K-Means loop in 2D or 3D for Part 1 and Part 2 respectively. Each cluster should have a unique color for the members (ie observations associated with a cluster center). Highlight the cluster centers using a different icon than for the other observations (ie a filled square instead of a filled circle or a larger size, etc.).

For Part 1 you can assume the input feature points are 2D, for Part 2 you can assume the input feature points are 3D and for Part 3 the input feature points are D-dimensional.

### **What to turn in?**

Three items to turn in:

1. You should turn in your code git repository as a zip file.
2. You also need to submit a short report in PDF format, with your implementation details, sample output and performance comparison between the three types of initializations (random real, random sample and K-Means++) comparing both speed and the clustering energy. 
3. You should upload a short animation (less than 4 minutes in length) of your K-Means interactive algorithm showing updates and user interaction (ie mp4, ffmpeg movie) for both Part 1 and Part 2.

### **Part 1: Implement interactive K-Means clustering with 2D visualization (50 points)**

Specifications for Part 1 include the following:

1. Use Euclidean (L2) feature distance computation. Include support for more than one feature distance function. For example, L1 for real features or Hamming distance for binary features.
2. Let the user specify K, the number of desired clusters.
3. Support the following three methods for initializing the cluster centers. Allow the user to select one of the desired initialization methods.
    - [ ] (a) random real - any real number x, y location for the seed within the sample space
    - [ ] (b) random sample - randomly select one of the observations
    - [ ] (c) K-Means++ distance based (D^2) careful seeding.
4. Use Qt to visualize the data and K-Means clustering using 2D point rendering. User interaction features include:
    - [ ] 1. Visualize the evolution of the class membership of each point using colors for each iteration of K-Means loop and with cluster centers marked.
    - [ ] 2. Allow the user to step thru the animation one iteration at a time or using a speed control; otherwise the updates/animation will be too fast.
    - [ ] 3. Allow zooming and panning to move around the 2D world.
    - [ ] 4. Allow the user to change the size of the 2D points.
5. Draw the updated centroid (mean) of each cluster, then draw the updated class membership value/color for each observation sample.
    - [ ] Allow the user to step thru the updates to interactively see which feature points change their class membership.
    - [ ] Allow the user to go backward one step in the animation.
6. Animate/iterate until the class labels do not change or the maximum iterations has been reached.
7. Read the feature points from an input file or auto generate synthetic test datasets with Gaussian distributions. The first line in the ASCII text file gives the value of N (number of feature points), D (dimensionality of the each feature vector), list of samples (ie feature vectors) one per line. Use several test cases:
    - [ ] 1. 15 dots in 2D (K=2 or K=3), 2D GMM (K=6); see slides (9 and 10) for the 15 dot example.
    - [ ] 2. Uniform distribution of points (no real clusters) as shown in Slide 11 left "Pancake" figure.
    - [ ] 3. Elliptical gaussian distribution of points as shown in Slide 11 right "Scissors or X" figure.
    - [ ] 4. Synthetic dataset spiral and rings as shown in Slides 12 and 13.
    - [ ] 5. Synthetic dataset 1 (K=4, D=2, N=400): 4 “real” centers at the corners of a square side length 250. Add point from Gaussian with variance 1, for both x and y, centered at each center for well-separated “ideal” clusters.
    6. Synthetic dataset 2 (K=4, D=2, N=400): 4 “real” centers at the corners of a square side length 250. Add point from Gaussian with variance 1 in x and variance 4 in y, centered at each center for elliptical clusters.
    7. Synthetic dataset 3 overlapping clusters like in Slide 14.
    8. (Optional synthetic data used in K-Means++) See slides 12 and 13 for Norm-10 and Norm-25 sample dataset descriptions. For example, Synthetic dataset Norm-10 (K=10, D=5, N=10,000): 10 “real” centers uniformly from hypercube of side length 500. Add point from Gaussian with variance 1, centered at each center for well-separated “ideal” clusters. This would be useful for timing.
8. Compare the speed and energy value of K-Means for three cases:
    - (a) random real,
    - (b) random sample,
    - (c) K-Means++ initialization (D2 weighting).
9. Include checks for degenerate conditions defined in the slides including Slide 15 (K=0, K=1, N=0, K>N, N identical points, sets of identical points).
10. (Optional 5 Bonus points) Let the user click on a cluster mean and **re-cluster** all of these observations into K2 clusters (where K2 can be different from K)
11. (Optional 5 Bonus points) When D>2 (or D>3) allow the user to select the two (or three) axes for display. If the number of features is very large then randomly select the 2 (or 3) feature dimensions to project onto (2-D or 3-D subspace) and display the clustering visualization. More informative subspaces are described below in the Optional Part 3.

### **Part 2: Implement interactive K-Means clustering with 3D visualization (50 points)**

Specifications for Part 2 include the same above items as Part 1, plus several additional requirements.

The input data sample points or observations are now 3D. Of course in the general case the observations can always be higher dimensional but you do not need to explicitly support D-dimensional features -- see Item 11 in Part 1 and Part 3 below for more details on D-dimensional sample space or feature vectors.

Use Qt to visualize the data and K-Means clustering using 3D point rendering. For interactive visualization the display is 3D and the user should be able to rotate, zoom and pan through the coordinate display ie point cloud visualization during the K-Means animation to view the points from different angles.

Allow the user to change the thickness of the 3D points.

### ***Optional Part 3:* Interactively visualize D-dimensional K-Means clustering using sub-space projections like PCA (Principal Component Analysis), tSNE (t-Distributed Stochastic Neighbor Embedding) or UMAP (Uniform Manifold Approximation and Projection). (50 Bonus points)**
