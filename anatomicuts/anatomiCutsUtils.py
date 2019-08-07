import csv
import os
import math
from scipy import stats
import numpy as np
import nibabel as nib
import os.path
import scipy.spatial
import glob
import subprocess
import pandas as pd
from nipy.modalities.fmri.glm import GeneralLinearModel
#import matplotlib.pyplot as plt

import freesurfer as fs
def getCorrespondingClusters(correspondance,order=True):
		corr=dict()
		distances=dict()
		indeces=dict()
		ind=0
		with open(correspondance, 'r') as csvfile:
				corReader= csv.reader(csvfile, delimiter=',', quotechar='|')	
				header=True
				for row in corReader:
					if not header and len(row)>1:
						if order:
							corr[row[1]]=row[0]
							indeces[ind]=row[1]
							distances[row[1]]=float(row[2])
						else:
							corr[row[0]]=row[1]
							indeces[ind]=row[0]
							distances[row[0]]=float(row[2])
						ind+=1
					else:
						header=False
		return corr, distances, indeces

def averageCorrespondingClusters(correspondences, imagesFolder, outputFolder,  clusterIndeces):
		averages=dict()
		norm=dict()
		for s_i, correspondance in enumerate(correspondences):
				try:
					for clusterIndex in clusterIndeces:
						corr, distances, indeces =  getCorrespondingClusters(correspondance, True)
						#print(correspondance)
						image=imagesFolder[s_i]+""+indeces[clusterIndex]+".nii.gz"
						im =nib.load(image)
						b= im.get_data()
						b = b/b.max()
						b = np.ceil(b) 
						if clusterIndex in averages:
							b += averages[clusterIndex].get_data() 
							averages[clusterIndex]=nib.Nifti1Image(b, averages[clusterIndex].get_affine())
							norm[clusterIndex]+=1
						else:
							averages[clusterIndex] = nib.Nifti1Image(b, im.get_affine())    
							norm[clusterIndex]=1
				except Exception as e:
					print(str(e))

		for clusterIndex in clusterIndeces:
				directory=outputFolder
				if not os.path.exists(directory):
					os.makedirs(directory)
				data=averages[clusterIndex].get_data()/norm[clusterIndex]
				nib.Nifti1Image(data, averages[clusterIndex].get_affine()).to_filename(directory+"/"+str(clusterIndex)+'.nii.gz')
				print ("saving",directory+"/"+str(clusterIndex)+'.nii.gz')

def readTree(numNodes, histogramFile,header=True):
    almostFoundAllClusters=False
    foundAllClusters=False
    nodes_childs=dict()
    whos_dad=dict()
    
    with open(histogramFile, 'r') as csvfile:
        reader = csv.reader(csvfile, delimiter=',', quotechar='|')	
        clusters=set()

        for row in reader:
            if not header:
                if not foundAllClusters:
                    try:
                        if row[0] in clusters:
                            clusters.remove(row[0])
                        clusters.add(row[1])
                        if len(clusters)==numNodes :
                            foundAllClusters=True
                            for i in clusters:
                                nodes_childs[i]=[]
                            
                    except:
                        #print ("header")
                        None
                else:
                    if row[0] in whos_dad :
                        dad = whos_dad[row[0]]
                        if row[0] in nodes_childs[dad]:
                            nodes_childs[dad].remove(row[0])
                        nodes_childs[dad].append(row[1])
                        whos_dad[row[1]]=dad	
                    else:
                        nodes_childs[row[0]].append(row[1])
                        whos_dad[row[1]]=row[0]
            else:
                header=False
            
    return nodes_childs, whos_dad

def groupAnalysis( headers, cols , groups_classification, classification_columns, clustersToAnalyze, subjects_dir, target_subject, delimiter=",", groupA=[0], groupB=[1], lenght, std):
	ac_folder=f"dmri.ac/{lenght}/{std}"
	#with open(groups_classification) as f:
	#	groups_cat = dict(filter(None, csv.reader(f, delimiter=',')))

	indeces=[]
	groups_cat = {row[classification_columns[0]] : row[classification_columns[1]] for _, row in pd.read_csv(groups_classification, delimiter=delimiter).iterrows()}
	significant_childs=set()	
	for c_i, clusterNum in enumerate(clustersToAnalyze):
		
		childs, dads = readTree(clusterNum, f"{subjects_dir}/{target_subject}/{ac_folder}/HierarchicalHistory.csv")
		#print(childs, dads)
		order_nodes= pd.read_csv(f"{subjects_dir}/{target_subject}/{ac_folder}/measures/{target_subject}_{target_subject}_c{clusterNum}.csv",delimiter=",", header=0,usecols=[0])
		#print(order_nodes["Cluster"][0])
		ys=[]
		for a in headers:
			ys.append([])
			
		for i in range(clusterNum):
			for j in range(len(headers)):
				ys[j].append([])
		X=[]
		
		for s in groups_cat.keys():
			subject=glob.glob(f'{subjects_dir}/{s}*')[0].split("/")[-1]
			measures=f"{subjects_dir}/{subject}/{ac_folder}/measures/{target_subject}_{subject}_c{clusterNum}.csv"

			data = pd.read_csv(measures,delimiter=",", header=0, names=headers, usecols=cols)
			#print(measures)
			if len(data[headers[0]])>= clusterNum:
				
				if int(groups_cat[s]) in groupA:
					X=np.append(X,[1, 0])
				elif int(groups_cat[s]) in groupB:
					X=np.append(X,[0, 1])
				
				if int(groups_cat[s]) in groupA+groupB:
					for i,h in enumerate(headers):
						for j in range(clusterNum):
							ys[i][j].append(data[h][j])

		X= np.array(X).reshape(len(ys[0][0]),2)
		for i, m  in enumerate(headers):
			Y=ys[i][0]
			for j in range(1,len(ys[i])):
				#print(np.shape(ys[i][j]))
				Y = np.vstack((Y,[ys[i][j]]))
			
			Y=np.array(Y).transpose()
			#print(np.shape(Y))
			cval = np.hstack((-1, 1))
			model = GeneralLinearModel(X)
			model.fit(Y)
			p_vals = model.contrast(cval).p_value() # z-transformed statistics
			#print( p_vals)
			for index, p in enumerate(p_vals):
				if p*len(ys[0]) <0.05:
					if len(childs[str(order_nodes["Cluster"][index])]) ==0:
						significant_childs.add(str(order_nodes["Cluster"][index]))
					else:
						for c in childs[str(order_nodes["Cluster"][index])]:
							significant_childs.add(c)
					
				if clusterNum == clustersToAnalyze[-1] and p<0.05 and str(order_nodes["Cluster"][index]) in significant_childs:
					print(m,index,p, p,str(order_nodes["Cluster"][index]))
					indeces.append(index)
					
				 
			cval = np.hstack((1, -1))
			model = GeneralLinearModel(X)
			model.fit(Y)
			p_vals = model.contrast(cval).p_value() # z-transformed statistics
			#print( p_vals)
			for index, p in enumerate(p_vals):
				if p*len(ys[0]) <0.05:
					#print(index,p, p*len(ys[0])*4,str(order_nodes["Cluster"][index])) 
					if len(childs[str(order_nodes["Cluster"][index])]) ==0:
						significant_childs.add(str(order_nodes["Cluster"][index]))
					else:
						for c in childs[str(order_nodes["Cluster"][index])]:
							significant_childs.add(c)
		    #plt.show()
				if clusterNum == clustersToAnalyze[-1] and p<0.05 and str(order_nodes["Cluster"][index]) in significant_childs:
					print(m, index,p, p,str(order_nodes["Cluster"][index]))
					indeces.append(index)
	return indeces
					
def plotAverageMeasures( headers, cols , groups_classification, classification_columns, clustersToAnalyze, subjects_dir, target_subject, delimiter=",", groups=[[1],[2],[3]], lenght, std):
	groups_cat = {row[classification_columns[0]] : row[classification_columns[1]] for _, row in pd.read_csv(groups_classification, delimiter=delimiter).iterrows()}
	clusterNum=200
	order_nodes= pd.read_csv(f"{subjects_dir}/{target_subject}/dmri.ac/{lenght}/{std}/measures/{target_subject}_{target_subject}_c{clusterNum}.csv",delimiter=",", header=0,usecols=[0])
	measures=[]
	for g in groups:
		measures.append([])
	for gi,g in enumerate(groups):
		for a in headers:
			measures[gi].append([])
		
	for gi, g in enumerate(groups):
		for i in range(clusterNum):
			for j in range(len(headers)):
				measures[gi][j].append([])
	
	for s in groups_cat.keys():
		subject=glob.glob(f'{subjects_dir}/{s}*')[0].split("/")[-1]
		measuresFile=f"{subjects_dir}/{subject}/dmri.ac/{lenght}/{std}/measures/{target_subject}_{subject}_c{clusterNum}.csv"
		try:
			data = pd.read_csv(measuresFile,delimiter=",", header=0, names=headers, usecols=cols)
			#print(measures)
			if len(data[headers[0]])>= clusterNum:
				val  = [ i for i in range(len(groups))  if int(groups_cat[s]) in groups[i]]
				if len(val)>0:
					group=val[0]
					for i,h in enumerate(headers):
						for j in range(clusterNum):
							measures[group][i][j].append(data[h][j])
		except:
			print (measuresFile)
	for i in clustersToAnalyze:
		plt.figure()
		for gi, g in enumerate( groups):
			plt.violinplot(measures[gi][0][i], [gi],  showmeans=True )
		plt.savefig(f"{subjects_dir}/average/dmri.ac/{lenght}_{std}/"+str(i)+"_"+headers[0]+".png")		
	#plt.show()

def GA(classificationFile, classification_cols, target_subject, subjects_dir, groupA, groupB, lenght=45, std=5,clustersToAnalyze=[200], model="DTI", delimiter="," ):
	measures=['FA','MD','RD','AD']
	if model == 'DKI':
		measures=measures+['MK','RK','AK']

	for i, m in enumerate(measures):
		indeces = groupAnalysis(headers=["mean"+m],cols=[2+i*4], groups_classification=classificationFile, classification_columns=classification_cols,clustersToAnalyze=clustersToAnalyze,target_subject=target_subject,subjects_dir=subjects_dir, delimiter=delimiter, groupA=groupsInA, groupB=groupsInB, lenght, std)
		plotAverageMeasures(headers=["meanFA"],cols=[2], groups_classification=classificationFile, classification_columns=classification_cols,clustersToAnalyze=indeces,target_subject=target_subject,subjects_dir=subjects_dir, delimiter=delimiter, groups=[groupsA,groupB], lenght, std)



cta=[200]
parser = fs.ArgParser()
# Required
parser.add_argument('-f','--function',  required=True, help='Function to use: GA for group analysis.')
parser.add_argument('-m','--model',required=False,  help='Model, which could be DTI or DKI')
parser.add_argument('-cf','--classification_file', metavar='file', required=False, help='classification file')
parser.add_argument('-cc','--classification_columns',required=False,  help='classification columns')
parser.add_argument('-cta','--clusters_to_analyze',required=False,  help='Clusters to analyze')
parser.add_argument('-ts','--target_subject',required=False,  help='target subject')
parser.add_argument('-s','--subject_folder', required=False, help='subject folder')
parser.add_argument('-ga','--group_a',required=False,  help='group a')
parser.add_argument('-gb','--group_b',required=False,  help='group b')
parser.add_argument('-d','--delimiter',required=False,  help='delimiter for classification file')
parser.add_argument('-l','--lenght',required=False,  help='minimum lenght')
parser.add_argument('-std','--std',required=False,  help='standard deviation of clusters')


args = parser.parse_args()
if args.function == "GA":
	eval(args.function)(args.classification_file, args.classification_columns, args.target_subject, args.subjects_folder, args.group_a, args.group_b,args.lenght, args.std, clusters_to_analyze, args.model, args.delimiter)
"""
groupAnalysis(headers=["meanFA"],cols=[2], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=cta,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[3], groupB=[2])



groupAnalysis(headers=["meanFA"],cols=[2], groups_classification="/space/vault/7/users/vsiless/lilla/classification.csv", classification_columns=[0,1], clustersToAnalyze=[50,100, 150,200],target_subject="INF007",subjects_dir="/space/vault/7/users/vsiless/lilla/AnatomiCuts/babybold/")
#groupAnalysis(headers=["meanADC"],cols=[6], groups_classification="/space/vault/7/users/vsiless/lilla/classification.csv", classification_columns=[0,1], clustersToAnalyze=[50,100, 150,200],target_subject="INF007",subjects_dir="/space/vault/7/users/vsiless/lilla/AnatomiCuts/babybold/")
#groupAnalysis(headers=["meanRD"],cols=[10], groups_classification="/space/vault/7/users/vsiless/lilla/classification.csv", classification_columns=[0,1], clustersToAnalyze=[50,100, 150,200],target_subject="INF007",subjects_dir="/space/vault/7/users/vsiless/lilla/AnatomiCuts/babybold/")
#groupAnalysis(headers=["meanAD"],cols=[14], groups_classification="/space/vault/7/users/vsiless/lilla/classification.csv", classification_columns=[0,1],clustersToAnalyze=[50,100, 150,200],target_subject="INF007",subjects_dir="/space/vault/7/users/vsiless/lilla/AnatomiCuts/babybold/")



groupAnalysis(headers=["meanFA"],cols=[2], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=cta,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[3], groupB=[2])
groupAnalysis(headers=["meanMD"],cols=[6], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=cta,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[3], groupB=[2])
groupAnalysis(headers=["meanRD"],cols=[10], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=cta,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[3], groupB=[2])
groupAnalysis(headers=["meanAD"],cols=[14], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=cta,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[3], groupB=[2])
groupAnalysis(headers=["meanMK"],cols=[18], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=cta,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[3], groupB=[2])
groupAnalysis(headers=["meanRK"],cols=[22], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=cta,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[3], groupB=[2])
groupAnalysis(headers=["meanAK"],cols=[26], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=cta,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[3], groupB=[2])

print ("group3")
indeces= groupAnalysis(headers=["meanFA"],cols=[2], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=[200],target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[1], groupB=[3])
plotAverageMeasures(headers=["meanFA"],cols=[2], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])

indeces =  groupAnalysis(headers=["meanMD"],cols=[6], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=[200],target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[1], groupB=[3])
plotAverageMeasures(headers=["meanMD"],cols=[6], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])


indeces = indeces +groupAnalysis(headers=["meanRD"],cols=[10], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=[200],target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[1], groupB=[3])
plotAverageMeasures(headers=["meaniRD"],cols=[10], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])

indeces = indeces +groupAnalysis(headers=["meanAD"],cols=[14], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=[200],target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[1], groupB=[3])
plotAverageMeasures(headers=["meanAD"],cols=[14], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])

indeces = indeces +groupAnalysis(headers=["meanMK"],cols=[18], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=[200],target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[1], groupB=[3])
plotAverageMeasures(headers=["meanMK"],cols=[18], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])

indeces = indeces +groupAnalysis(headers=["meanRK"],cols=[22], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=[200],target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[1], groupB=[3])
plotAverageMeasures(headers=["meanRK"],cols=[22], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])

indeces = indeces +groupAnalysis(headers=["meanAK"],cols=[26], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=[200],target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groupA=[1], groupB=[3])
plotAverageMeasures(headers=["meanAK"],cols=[26], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])
"""
#print(indices)
#indeces=[41,10]
"""
plotAverageMeasures(headers=["meanFA"],cols=[2], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])
plotAverageMeasures(headers=["meanMD"],cols=[6], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])
plotAverageMeasures(headers=["meanAD"],cols=[10], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])
plotAverageMeasures(headers=["meanRD"],cols=[14], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])
plotAverageMeasures(headers=["meanAK"],cols=[18], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])
plotAverageMeasures(headers=["meanRK"],cols=[22], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])
plotAverageMeasures(headers=["meanMK"],cols=[26], groups_classification="/space/snoke/1/public/vivros/data/demos_fullID2.csv", classification_columns=[0,6],clustersToAnalyze=indeces,target_subject="6002_16_01192018",subjects_dir="/space/snoke/1/public/vivros/AnatomiCuts_l35/", delimiter=" ", groups=[[1],[2],[3]])
"""
"""ts="BAKP64e_nrecon-trio3_vb17"
#classFile="/space/snoke/1/public/vivros/labels.csv"
classFile="/autofs/space/snoke_001/public/vivros/hd_tracula/labels.csv"
folder="/autofs/space/snoke_001/public/vivros/hd_tracula/"
cta=[200]

indices = groupAnalysis(headers=["meanFA"],cols=[2], groups_classification=classFile, classification_columns=[0,1],clustersToAnalyze=cta,target_subject=ts,subjects_dir=folder, delimiter=" ", groupA=[8], groupB=[6])
indices = indices + groupAnalysis(headers=["meanMD"],cols=[6], groups_classification=classFile, classification_columns=[0,1],clustersToAnalyze=cta,target_subject=ts,subjects_dir=folder, delimiter=" ", groupA=[8], groupB=[6])
indices = indices + groupAnalysis(headers=["meanRD"],cols=[10], groups_classification=classFile, classification_columns=[0,1],clustersToAnalyze=cta,target_subject=ts,subjects_dir=folder, delimiter=" ", groupA=[8], groupB=[6])
indices = indices + groupAnalysis(headers=["meanAD"],cols=[14], groups_classification=classFile, classification_columns=[0,1],clustersToAnalyze=cta,target_subject=ts,subjects_dir=folder, delimiter=" ", groupA=[8], groupB=[6])


plotAverageMeasures(headers=["meanFA"],cols=[2], groups_classification=classFile, classification_columns=[0,1],clustersToAnalyze=indices,target_subject=ts,subjects_dir=folder, delimiter=" ", groups=[[8],[6]])
plotAverageMeasures(headers=["meanMD"],cols=[6], groups_classification=classFile, classification_columns=[0,1],clustersToAnalyze=indices,target_subject=ts,subjects_dir=folder, delimiter=" ", groups=[[8],[6]])
plotAverageMeasures(headers=["meanRD"],cols=[10], groups_classification=classFile, classification_columns=[0,1],clustersToAnalyze=indices,target_subject=ts,subjects_dir=folder, delimiter=" ", groups=[[8],[6]])
plotAverageMeasures(headers=["meanAD"],cols=[14], groups_classification=classFile, classification_columns=[0,1],clustersToAnalyze=indices,target_subject=ts,subjects_dir=folder, delimiter=" ", groups=[[8],[6]])

"""

