namespace mystl
{
    template <typename... Types>
    class tuple;
    
    template <typename Head>
    class tuple<Head>
    {
    private:
        Head head_;
        
    public:
        tuple() {};
        tuple(const Head &head) 
            : head_(head) {};
            
        auto _Get_head() const -> const Head&
        {
            return head_;
        };
    };
    
    template <typename Head, typename... Tail>
    class tuple<Head, Tail...> : public tuple<Head>
    {
    private:
        tuple<Tail...> tail_;
        
    public:
        tuple() : tuple<Head>() {};
        tuple(const Head &head, const tuple<Tail...> &tail)
            : tuple<Head>(head), tail_(tail) {};
            
        auto _Get_tail() const -> const tuple<Tail...>&
        {
            return tail_;
        };
    };
    
    template <size_t N>
    struct TupleGetter
    {
        template <typename Tuple>
        static auto apply(const Tuple &t)
        {
            return TupleGetter<N-1>::apply(t._Get_tail());
        };
    };
    
    template <>
    struct TupleGetter<0>
    {
        template <typename Tuple>
        static auto apply(const Tuple &t)
        {
            return t._Get_head();
        };
    };
    
    template <size_t N, typename Tuple>
    auto get(const Tuple &t)
    {
        return TupleGetter<N>::apply(t);
    };
};