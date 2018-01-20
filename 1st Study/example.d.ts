declare namespace std
{
	export class ReverseIterator<T, Source, Iterator, MyType> {}

	export class Entry<Key, T> {}
}

declare namespace std
{
	export interface IArrayContainer<T> {}

	export abstract class ArrayContainer<T, Source extends IArrayContainer<T>> 
		implements IArrayContainer<T>
	{
		public begin(): ArrayIterator<T, Source>;
		public end(): ArrayIterator<T, Source>;

		public rbegin(): ArrayReverseIterator<T, Source>;
		public rend(): ArrayReverseIterator<T, Source>;
	}

	export class ArrayIterator<T, Source extends IArrayContainer<T>>
	{
		public source(): Source;
	}
	export class ArrayReverseIterator<T, Source extends IArrayContainer<T>>
		extends ReverseIterator
		<
			T, Source, 
			ArrayIterator<T, Source>, 
			ArrayReverseIterator<T, Source>
		> {}
}

declare namespace std
{
	export interface IMapContainer<Key, T> {}

	export abstract class MapContainer<Key, T, Source extends IMapContainer<Key, T>>
	{
		public begin(): MapIterator<Key, T, Source>;
		public end(): MapIterator<Key, T, Source>;

		public rbegin(): MapReverseIterator<Key, T, Source>;
		public rend(): MapReverseIterator<Key, T, Source>;
	}

	export class MapIterator<Key, T, Source extends IMapContainer<Key, T>>
		implements IMapContainer<Key, T>
	{
		public source(): MapContainer<Key, T, Source>;
	}
	export class MapReverseIterator<Key, T, Source extends IMapContainer<Key, T>>
		extends ReverseIterator
		<
			Entry<Key, T>, 
			MapContainer<Key, T, Source>, 
			MapIterator<Key, T, Source>, 
			MapReverseIterator<Key, T, Source>
		> {}
}