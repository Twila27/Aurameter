#pragma once


enum BufferUsage //Hints to GPU for optimizations.
{
	STATIC_DRAW, //Buffer data will be set once.
	DYNAMIC_DRAW, //Buffer data will be set sometimes.
	STREAM_DRAW, //Buffer data will be set potentially per-use.
	USE_LAST_USAGE
};


class VertexBuffer;
typedef VertexBuffer IndexBuffer;


class VertexBuffer 
{
public:

	VertexBuffer( unsigned int numElements, unsigned int sizeOfElementInBytes, BufferUsage usage, const void* data );
	~VertexBuffer();
	inline unsigned int GetBufferID() const { return m_bufferID; }
	inline size_t GetElementSize() const { return m_elementSize; }
	inline unsigned int GetNumElements() const { return m_numElements; }
	inline BufferUsage GetBufferUsage() const { return m_bufferUsage; }
	inline unsigned int GetBufferSize() const { return m_elementSize * m_numElements; }
	void UpdateBuffer( const void* data, unsigned int numElements, unsigned int sizeOfElementInBytes, BufferUsage bufferUsage = USE_LAST_USAGE ); //void* because array can be of vertex class types, or uint indices.


private:

	unsigned int m_bufferID;
	unsigned int m_numElements;
	unsigned int m_elementSize; //Use VertexDefinition instead?
	BufferUsage m_bufferUsage;
};
