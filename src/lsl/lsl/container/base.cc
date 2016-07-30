#include <boost/format.hpp>

namespace LSL {

template < class T >
ContainerBase<T>::ContainerBase()
{
}

template < class T >
void ContainerBase<T>::Add( PointerType item )
{
    m_map[item->key()] = item;
    m_seekpos = SEEKPOS_INVALID;
}

template < class T >
typename ContainerBase<T>::PointerType ContainerBase<T>::Add( ItemType* item )
{
    PointerType p( item );
    Add( p );
    return p;
}

template < class T >
void ContainerBase<T>::Remove( const KeyType& index )
{
    typename ContainerBase<T>::MapType::iterator
        it = m_map.find( index );
    const bool found = it != m_map.end();
    if ( found ) {
        m_map.erase( it );
        m_seekpos = SEEKPOS_INVALID;
    }
}

template < class T >
const typename ContainerBase<T>::PointerType ContainerBase<T>::Get( const KeyType& index ) const
{
    typename ContainerBase<T>::MapType::const_iterator
        it = m_map.find( index );
    if ( it == m_map.end() )
        throw MissingItemException( index );
    return it->second;
}

template < class T >
typename ContainerBase<T>::PointerType ContainerBase<T>::Get( const KeyType& index )
{
    typename ContainerBase<T>::MapType::iterator
        it = m_map.find( index );
    if ( it == m_map.end() )
        throw MissingItemException( index );
    return it->second;
}

template < class T >
bool ContainerBase<T>::Exists( const KeyType& index ) const
{
    return m_map.find( index ) != m_map.end();
}

template < class T >
typename ContainerBase<T>::MapType::size_type ContainerBase<T>::size() const
{
    return m_map.size();
}

template < class T >
ContainerBase<T>::MissingItemException::MissingItemException( const typename ContainerBase<T>::KeyType& key )
    : std::runtime_error( (boost::format( "No %s found in list for item with key %s" ) % T::className() % key).str() )
{}

template < class T >
ContainerBase<T>::MissingItemException::MissingItemException( const typename ContainerBase<T>::MapType::size_type& index )
    : std::runtime_error( (boost::format( "No %s found in list for item with pseudo index %s" ) % T::className() % index).str() )
{}

template < class T >
const typename ContainerBase<T>::ConstPointerType
ContainerBase<T>::At( const typename ContainerBase<T>::MapType::size_type index) const
{
    if ((m_seekpos == SEEKPOS_INVALID) || (m_seekpos > index)) {
        m_seek = m_map.begin();
        m_seekpos = 0;
    }
    std::advance( m_seek, index - m_seekpos );
    m_seekpos = index;
    return m_seek->second;
}

template < class T >
const typename ContainerBase<T>::PointerType
ContainerBase<T>::At( const typename ContainerBase<T>::MapType::size_type index)
{
    if ((m_seekpos == SEEKPOS_INVALID) || (m_seekpos > index)) {
        m_seek = m_map.begin();
        m_seekpos = 0;
    }
    std::advance( m_seek, index - m_seekpos );
    m_seekpos = index;
    return m_seek->second;
}

template < class T >
typename ContainerBase<T>::ConstVectorType
ContainerBase<T>::Vectorize() const
{
    std::vector< ConstPointerType > ret;
    for( typename MapType::const_iterator it = m_map.begin(); it != m_map.end(); ++it ) {
        ret.push_back( it->second );
    }
    return ret;
}

template < class T >
typename ContainerBase<T>::VectorType
ContainerBase<T>::Vectorize()
{
    std::vector< PointerType > ret;
    for( typename MapType::iterator it = m_map.begin(); it != m_map.end(); ++it ) {
        ret.push_back( it->second );
    }
    return ret;
}

template < class PtrType >
struct map_data_compare
{
    const PtrType ptr_;
    map_data_compare( const PtrType ptr ) : ptr_(ptr) {}
    template < class PairType >
    bool operator() ( const PairType& pair ) const
    {
        return pair.second == ptr_;
    }
};

template < class T >
bool ContainerBase<T>::Exists( const ConstPointerType ptr ) const
{
    typedef ContainerBase<T>::ConstPointerType
            PP;
    return end() != std::find_if( begin(), end(),
                                  map_data_compare<PP>(ptr) );
}

}
