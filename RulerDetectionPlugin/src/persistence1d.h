/*! \file persistence1d.hpp
    Actual code.
*/

// Code by Yeara Kozlov and Tino Weinkauf, Max Planck Institute for Informatics, Saarbrücken, Germany
// https://people.mpi-inf.mpg.de/~weinkauf/notes/persistence1d.html

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

#define NO_COLOR -1
#define RESIZE_FACTOR 20
#define MATLAB_INDEX_FACTOR 1

namespace p1d 
{

/** Used to sort data according to its absolute value and refer to its original index in the Data vector.

	A collection of TIdxAndData is sorted according to its data value (if values are equal, according
	to indices). The index allows access back to the vertex in the Data vector. 
*/
struct TIdxAndData
{
	TIdxAndData():Idx(-1),Data(0){}

	bool operator<(const TIdxAndData& other) const
	{
		if (Data < other.Data) return true;
		if (Data > other.Data) return false;
		return (Idx < other.Idx);
	}

	///The index of the vertex within the Data vector. 
	int Idx;

	///Vertex data value from the original Data vector sent as an argument to RunPersistence.
	float Data;
};


/*! Defines a component within the data domain. 
	A component is created at a local minimum - a vertex whose value is smaller than both of its neighboring 
	vertices' values.
*/
struct TComponent
{
	///A component is defined by the indices of its edges.
	///Both variables hold the respective indices of the vertices in Data vector.
	///All vertices between them are considered to belong to this component.
	int LeftEdgeIndex;
	int RightEdgeIndex;

	///The index of the local minimum within the component as longs as its alive. 
	int MinIndex;

	///The value of the Data[MinIndex].
	float MinValue; //redundant, but makes life easier

	///Set to true when a component is created. Once components are merged,
	///the destroyed component Alive value is set to false. 
	///Used to verify correctness of algorithm.
	bool Alive;
};


/** A pair of matched local minimum and local maximum
	that define a component above a certain persistence threshold.
	The persistence value is their (absolute) data difference.
*/
struct TPairedExtrema
{
	///Index of local minimum, as per Data vector.
	int MinIndex;

	///Index of local maximum, as per Data vector. 
	int MaxIndex;

	///The persistence of the two extrema.
	///Data[MaxIndex] - Data[MinIndex]		 
	///Guaranteed to be >= 0.
	float Persistence;	

	bool operator<(const TPairedExtrema& other) const
	{
		if (Persistence < other.Persistence) return true;
		if (Persistence > other.Persistence) return false;
		return (MinIndex < other.MinIndex);
	}
};



/*! Finds extrema and their persistence in one-dimensional data.

	Local minima and local maxima are extracted, paired,
	and sorted according to their persistence.
	The global minimum is extracted as well.

	We assume a connected one-dimensional domain.
	Think of "data on a line", or a function f(x) over some domain xmin <= x <= xmax.
*/
class Persistence1D
{
public:
	Persistence1D()
	{
	}

	~Persistence1D()
	{
	}
			
	/*!
		Call this function with a vector of one dimensional data to find extrema features in the data.
		The function runs once for, results can be retrieved with different persistent thresholds without
		further data processing.		
						
		Input data vector is assumed to be of legal size and legal values.
		
		Use PrintResults, GetPairedExtrema or GetExtremaIndices to get results of the function.

		@param[in] InputData Vector of data to find features on, ordered according to its axis.
	*/
	bool RunPersistence(const std::vector<float>& InputData)
	{	
		Data = InputData; 
		Init();

		//If a user runs this on an empty vector, then they should not get the results of the previous run.
		if (Data.empty()) return false;

		CreateIndexValueVector();
		Watershed();
		SortPairedExtrema();
#ifdef _DEBUG
		VerifyAliveComponents();	
#endif
		return true;
	}



	/*!
		Prints the contents of the TPairedExtrema vector.
		If called directly with a TPairedExtrema vector, the global minimum is not printed.

		@param[in] pairs	Vector of pairs to be printed. 
	*/	
	void PrintPairs(const std::vector<TPairedExtrema>& pairs) const 
	{
		for (std::vector<TPairedExtrema>::const_iterator it = pairs.begin(); 
			it != pairs.end(); it++)
		{
			std::cout	<< "Persistence: " << (*it).Persistence
						<< " minimum index: " << (*it).MinIndex
						<< " maximum index: " << (*it).MaxIndex
						<< std::endl;
		}
	}

	/*!
		Prints the global minimum and all paired extrema whose persistence is greater or equal to threshold. 
		By default, all pairs are printed. Supports Matlab indexing.
		
		@param[in] threshold		Threshold value for pair persistence.
		@param[in] matlabIndexing	Use Matlab indexing for printing.
	*/	
	void PrintResults(const float threshold = 0.0, const bool matlabIndexing = false) const
	{
		if (threshold < 0)
		{
			std::cout << "Error. Threshold value must be greater than or equal to 0" << std::endl;
		}
		if (threshold==0 && !matlabIndexing)
		{
			PrintPairs(PairedExtrema);
		}
		else 
		{
			std::vector<TPairedExtrema> pairs;
			GetPairedExtrema(pairs, threshold, matlabIndexing);
			PrintPairs(pairs);
		}

		std::cout << "Global minimum value: " << GetGlobalMinimumValue() 
				  << " index: " << GetGlobalMinimumIndex(matlabIndexing) 
				  << std::endl;
	}


	/*!
		Use this method to get the results of RunPersistence.
		Returned pairs are sorted according to persistence, from least to most persistent. 
		
		@param[out]	pairs			Destination vector for PairedExtrema
		@param[in]	threshold		Minimal persistence value of returned features. All PairedExtrema 
									with persistence equal to or above this value will be returned. 
									If left to default, all PairedMaxima will be returned.
		
		@param[in] matlabIndexing	Set this to true to change all indices of features to Matlab's 1-indexing.
	*/
	bool GetPairedExtrema(std::vector<TPairedExtrema> & pairs, const float threshold = 0, const bool matlabIndexing = false) const
	{
		//make sure the user does not use previous results that do not match the data
		pairs.clear();

		if (PairedExtrema.empty() || threshold < 0.0) return false;

		std::vector<TPairedExtrema>::const_iterator lower_bound = FilterByPersistence(threshold);

		if (lower_bound == PairedExtrema.end()) return false;
		
		pairs = std::vector<TPairedExtrema>(lower_bound, PairedExtrema.end());
		
		if (matlabIndexing) //match matlab indices by adding one
		{
			for (std::vector<TPairedExtrema>::iterator p = pairs.begin(); p != pairs.end(); p++)
			{
				(*p).MinIndex += MATLAB_INDEX_FACTOR;
				(*p).MaxIndex += MATLAB_INDEX_FACTOR;			
			}
		}
		return true;
	}

	/*!
	Use this method to get two vectors with all indices of PairedExterma.
	Returns false if no paired features were found.
	Returned vectors have the same length.
	Overwrites any data contained in min, max vectors.

	@param[out] min				Vector of indices of paired local minima. 
	@param[out]	max				Vector of indices of paired local maxima.
	@param[in]	threshold		Return only indices for pairs whose persistence is greater than or equal to threshold. 
	@param[in]	matlabIndexing	Set this to true to change all indices to match Matlab's 1-indexing.
*/
	bool GetExtremaIndices(std::vector<int> & min, std::vector<int> & max, const float threshold = 0, const bool matlabIndexing = false) const
	{
		//before doing anything, make sure the user does not use old results
		min.clear();
		max.clear();
				
		if (PairedExtrema.empty() || threshold < 0.0) return false;
		
		min.reserve(PairedExtrema.size());
		max.reserve(PairedExtrema.size());
		
		int matlabIndexFactor = 0;
		if (matlabIndexing) matlabIndexFactor = MATLAB_INDEX_FACTOR;

		std::vector<TPairedExtrema>::const_iterator lower_bound = FilterByPersistence(threshold);

		for (std::vector<TPairedExtrema>::const_iterator p = lower_bound; p != PairedExtrema.end(); p++)
		{
			min.push_back((*p).MinIndex + matlabIndexFactor);
			max.push_back((*p).MaxIndex + matlabIndexFactor);
		}
		return true;
	}
	/*!
		Returns the index of the global minimum. 
		The global minimum does not get paired and is not returned 
		via GetPairedExtrema and GetExtremaIndices.
	*/
	int GetGlobalMinimumIndex(const bool matlabIndexing = false) const
	{
		if (Components.empty()) return -1;

		assert(Components.front().Alive);
		if (matlabIndexing)
		{
			return Components.front().MinIndex + 1;
		}

		return Components.front().MinIndex;
	}

	/*!
		Returns the value of the global minimum. 
		The global minimum does not get paired and is not returned 
		via GetPairedExtrema and GetExtremaIndices.
	*/
	float GetGlobalMinimumValue() const
	{
		if (Components.empty()) return 0;

		assert(Components.front().Alive);
		return Components.front().MinValue;
	}
	/*!
		Runs basic sanity checks on results of RunPersistence: 
		- Number of unique minima = number of unique maxima - 1 (Morse property)
		- All returned indices are unique (no index is returned as two extrema)
		- Global minimum is within domain indices or at default value	
		- Global minimum is not returned as any other extrema.
		- Global minimum is not paired.
		
		Returns true if run results pass these sanity checks.
	*/
	bool VerifyResults() 
	{
		bool flag = true; 
		std::vector<int> min, max;
		std::vector<int> combinedIndices;
		
		GetExtremaIndices(min, max);

		int globalMinIdx = GetGlobalMinimumIndex();
				
		std::sort(min.begin(), min.end());
		std::sort(max.begin(), max.end());
		combinedIndices.reserve(min.size() + max.size());		
		std::set_union(min.begin(), min.end(), max.begin(), max.end(), std::inserter(combinedIndices, combinedIndices.begin())); 
		
		//check the combined unique indices are equal to size of min and max
		if (combinedIndices.size() != (min.size() + max.size()) ||
			 std::binary_search(combinedIndices.begin(), combinedIndices.end(), globalMinIdx) == true)
		{
		   flag = false;
		}

		if ((globalMinIdx > (int)Data.size()-1) || (globalMinIdx < -1)) flag = false;
		if (globalMinIdx == -1 && min.size() != 0) flag = false;
		
		std::vector<int>::iterator minUniqueEnd = std::unique(min.begin(), min.end());
		std::vector<int>::iterator maxUniqueEnd = std::unique(max.begin(), max.end());
				
		if (minUniqueEnd != min.end() ||
			maxUniqueEnd != max.end() ||
			(minUniqueEnd - min.begin()) != (maxUniqueEnd - max.begin()))
		{
			flag = false;
		}

		return flag;
	}

protected:
	/*!
		Contain a copy of the original input data.
	*/
	std::vector<float> Data;
	
	
	/*!
		Contains a copy the value and index pairs of Data, sorted according to the data values.
	*/
	std::vector<TIdxAndData> SortedData; 


	/*!
		Contains the Component assignment for each vertex in Data. 
		Only edges of destroyed components are updated to the new component color.
		The Component values in this vector are invalid at the end of the algorithm.
	*/
	std::vector<int> Colors;		//need to init to empty


	/*!
		A vector of Components. 
		The component index within the vector is used as its Colors in the Watershed function.
	*/
	std::vector<TComponent> Components;


	/*!
		A vector of paired extrema features - always a minimum and a maximum.
	*/
	std::vector<TPairedExtrema> PairedExtrema;
	
		
	unsigned int TotalComponents;	//keeps track of component vector size and newest component "color"
	bool AliveComponentsVerified;	//Index of global minimum in Data vector. This minimum is never paired.
	
	
	/*!
		Merges two components by doing the following:

		- Destroys component with smaller hub (sets Alive=false).
		- Updates surviving component's edges to span the destroyed component's region.
		- Updates the destroyed component's edge vertex colors to the survivor's color in Colors[].

		@param[in] firstIdx,secondIdx	Indices of components to be merged. Their order does not matter. 
	*/
	void MergeComponents(const int firstIdx, const int secondIdx)
	{
		int survivorIdx, destroyedIdx;
		//survivor - component whose hub is bigger
		if (Components[firstIdx].MinValue < Components[secondIdx].MinValue)
		{
			survivorIdx = firstIdx;
			destroyedIdx = secondIdx; 
		}
		else if (Components[firstIdx].MinValue > Components[secondIdx].MinValue)
		{
			survivorIdx = secondIdx;
			destroyedIdx = firstIdx; 
		}
		else if (firstIdx < secondIdx) // Both components min values are equal, destroy component on the right
										// This is done to fit with the left-to-right total ordering of the values
		{
			survivorIdx = firstIdx;
			destroyedIdx = secondIdx;
		}
		else  
		{
			survivorIdx = secondIdx;
			destroyedIdx = firstIdx;
		}

		//survivor and destroyed are decided, now destroy!
		Components[destroyedIdx].Alive = false;

		//Update the color of the edges of the destroyed component to the color of the surviving component.
		Colors[Components[destroyedIdx].RightEdgeIndex] = survivorIdx;
		Colors[Components[destroyedIdx].LeftEdgeIndex] = survivorIdx;

		//Update the relevant edge index of surviving component, such that it contains the destroyed component's region.
		if (Components[survivorIdx].MinIndex > Components[destroyedIdx].MinIndex) //destroyed index to the left of survivor, update left edge
		{
			Components[survivorIdx].LeftEdgeIndex = Components[destroyedIdx].LeftEdgeIndex;
		}
		else 
		{
			Components[survivorIdx].RightEdgeIndex = Components[destroyedIdx].RightEdgeIndex;		
		}
	}
	
	/*!
		Creates a new PairedExtrema from the two indices, and adds it to PairedFeatures.

		@param[in] firstIdx, secondIdx Indices of vertices to be paired. Order does not matter. 
	*/
	void CreatePairedExtrema(const int firstIdx, const int secondIdx)
	{
		TPairedExtrema pair; 
		
		//There might be a potential bug here, todo (we're checking data, not sorted data)
		//example case: 1 1 1 1 1 1 -5 might remove if after else
		if (Data[firstIdx] > Data[secondIdx])
		{
			pair.MaxIndex = firstIdx; 
			pair.MinIndex = secondIdx;
		}
		else if (Data[secondIdx] > Data[firstIdx])
		{
			pair.MaxIndex = secondIdx; 
			pair.MinIndex = firstIdx;
		}
		//both values are equal, choose the left one as the min
		else if (firstIdx < secondIdx)
		{
			pair.MinIndex = firstIdx;
			pair.MaxIndex = secondIdx;
		}
		else 
		{
			pair.MinIndex = secondIdx;
			pair.MaxIndex = firstIdx;
		}
				
		pair.Persistence = Data[pair.MaxIndex] - Data[pair.MinIndex];

#ifdef _DEBUG
		assert(pair.Persistence >= 0);
#endif
		if (PairedExtrema.capacity() == PairedExtrema.size()) 
		{
			PairedExtrema.reserve(PairedExtrema.size() * 2 + 1);
		}

		PairedExtrema.push_back(pair);
	}


	// Changing the alignment of the next Doxygen comment block breaks its formatting.

	/*! Creates a new component at a local minimum. 
		
	Neighboring vertices are assumed to have no color.
	- Adds a new component to the components vector, 
	- Initializes its edges and minimum index to minIdx.
	- Updates Colors[minIdx] to the component's color.

	@param[in]	minIdx Index of a local minimum. 
	*/
	void CreateComponent(const int minIdx)
	{
		TComponent comp;
		comp.Alive = true;
		comp.LeftEdgeIndex = minIdx;
		comp.RightEdgeIndex = minIdx;
		comp.MinIndex = minIdx;
		comp.MinValue = Data[minIdx];

		//place at the end of component vector and get the current size
		if (Components.capacity() <= TotalComponents)
		{	
			Components.reserve(2 * TotalComponents + 1);
		}

		Components.push_back(comp);
		Colors[minIdx] = TotalComponents;
		TotalComponents++;
	}


	/*!
		Extends the component's region by one vertex:
			
		- Updates the matching component's edge to dataIdx..
		- updates Colors[dataIdx] to the component's color.

		@param[in]	componentIdx	Index of component (the value of a neighboring vertex in Colors[]).
		@param[in] 	dataIdx			Index of vertex which the component is extended to.
	*/
	void ExtendComponent(const int componentIdx, const int dataIdx)
	{
#ifdef _DEUBG
		assert(Components[componentIdx].Alive == true)
#endif 

		//extend to the left
		if (dataIdx + 1 == Components[componentIdx].LeftEdgeIndex)
		{
			Components[componentIdx].LeftEdgeIndex = dataIdx;
		}
		//extend to the right
		else if (dataIdx - 1 == Components[componentIdx].RightEdgeIndex) 
		{
			Components[componentIdx].RightEdgeIndex = dataIdx;
		}
		else
		{
#ifdef _DEUBG
			std::string errorMessage = "ExtendComponent: index mismatch. Data index: ";
			errorMessage += std::to_string((long long)dataIdx);
			throw (errorMessage);
#endif 
		}

		Colors[dataIdx] = componentIdx;
	}


	/*!
		Initializes main data structures used in class:
		- Sets Colors[] to NO_COLOR
		- Reserves memory for Components and PairedExtrema
	
		Note: SortedData is should be created before, separately, using CreateIndexValueVector()
	*/
	void Init()
	{
		SortedData.clear();
		SortedData.reserve(Data.size());
		
		Colors.clear();
		Colors.resize(Data.size());
		std::fill(Colors.begin(), Colors.end(), NO_COLOR);
		
		int vectorSize = (int)(Data.size()/RESIZE_FACTOR) + 1; //starting reserved size >= 1 at least
		
		Components.clear();
		Components.reserve(vectorSize);

		PairedExtrema.clear();
		PairedExtrema.reserve(vectorSize);

		TotalComponents = 0;
		AliveComponentsVerified = false;
	}


	/*!
		Creates SortedData vector.
		Assumes Data is already set.
	*/	
	void CreateIndexValueVector()
	{
		if (Data.size()==0) return;
				
		for (std::vector<float>::size_type i = 0; i != Data.size(); i++)
		{
			TIdxAndData dataidxpair; 

			//this is going to make problems
			dataidxpair.Data = Data[i]; 
			dataidxpair.Idx = (int)i; 

			SortedData.push_back(dataidxpair);
		}

		std::sort(SortedData.begin(), SortedData.end());
	}


	/*!
		Main algorithm - all of the work happen here.

		Use only after calling CreateIndexValueVector and Init functions.

		Iterates over each vertex in the graph according to their ordered values:
		- Creates a segment for each local minima
		- Extends a segment is data has only one neighboring component
		- Merges segments and creates new PairedExtrema when a vertex has two neighboring components. 
	*/
	void Watershed()
	{
		if (SortedData.size()==1)
		{
			CreateComponent(0);
			return;
		}

		for (std::vector<TIdxAndData>::iterator p = SortedData.begin(); p != SortedData.end(); p++)
		{
			int i = (*p).Idx;

			//left most vertex - no left neighbor
			//two options - either local minimum, or extend component
			if (i==0)
			{
				if (Colors[i+1] == NO_COLOR) 
				{
					CreateComponent(i);
				}
				else
				{
					ExtendComponent(Colors[i+1], i);  //in this case, local max as well
				}
				
				continue;
			}
			else if (i == Colors.size()-1) //right most vertex - look only to the left
			{
				if (Colors[i-1] == NO_COLOR) 
				{
					CreateComponent(i);
				}
				else
				{
					ExtendComponent(Colors[i-1], i);
				}				
				continue;
			}

			//look left and right
			if (Colors[i-1] == NO_COLOR && Colors[i+1] == NO_COLOR) //local minimum - create new component
			{
				CreateComponent(i);
			}
			else if (Colors[i-1] != NO_COLOR && Colors[i+1] == NO_COLOR) //single neighbor on the left - extnd
			{
				ExtendComponent(Colors[i-1], i);
			}
			else if (Colors[i-1] == NO_COLOR && Colors[i+1] != NO_COLOR) //single component on the right - extend
			{
				ExtendComponent(Colors[i+1], i);
			}
			else if (Colors[i-1] != NO_COLOR && Colors[i+1] != NO_COLOR) //local maximum - merge components
			{
				int leftComp, rightComp; 

				leftComp = Colors[i-1];
				rightComp = Colors[i+1]; 

				//choose component with smaller hub destroyed component
				if (Components[rightComp].MinValue < Components[leftComp].MinValue) //left component has smaller hub
				{
					CreatePairedExtrema(Components[leftComp].MinIndex, i);
				}
				else	//either right component has smaller hub, or hubs are equal - destroy right component. 
				{
					CreatePairedExtrema(Components[rightComp].MinIndex, i);
				}
					
				MergeComponents(leftComp, rightComp);
				Colors[i] = Colors[i-1]; //color should be correct at both sides at this point
			}
		}
	}


	/*!
		Sorts the PairedExtrema list according to the persistence of the features. 
		Orders features with equal persistence according the the index of their minima.
	*/
	void SortPairedExtrema()
	{
		std::sort(PairedExtrema.begin(), PairedExtrema.end());
	}


	/*!
		Returns an iterator to the first element in PairedExtrema whose persistence is bigger or equal to threshold. 
		If threshold is set to 0, returns an iterator to the first object in PairedExtrema.
		
		@param[in]	threshold	Minimum persistence of features to be returned.		
	*/
	std::vector<TPairedExtrema>::const_iterator FilterByPersistence(const float threshold = 0) const
	{		
		if (threshold == 0 || threshold < 0) return PairedExtrema.begin();

		TPairedExtrema searchPair; 
		searchPair.Persistence = threshold;
		searchPair.MaxIndex = 0; 
		searchPair.MinIndex = 0;
		return(lower_bound(PairedExtrema.begin(), PairedExtrema.end(), searchPair));
	}
	/*!
		Runs at the end of RunPersistence, after Watershed. 
		Algorithm results should be as followed: 
		- All but one components should not be Alive. 
		- The Alive component contains the global minimum.
		- The Alive component should be the first component in the Component vector
	*/
	bool VerifyAliveComponents()
	{
		//verify that the Alive component is component #0 (contains global minimum by definition)
		if ((*Components.begin()).Alive != true) 
		{		
				
#ifndef _DEBUG 
			return false;
#endif 
#ifdef _DEBUG
			throw "Error. Component 0 is not Alive, assumed to contain global minimum";
#endif
		}
		
		for (std::vector<TComponent>::const_iterator it = Components.begin()+1; it != Components.end(); it++)
		{
			if ((*it).Alive == true) 
			{
							
#ifndef _DEBUG 
				return false;
#endif 
#ifdef _DEBUG
				throw "Error. Found more than one alive component";
#endif
			}
		}
		
		return true;
	}
};
}
#endif