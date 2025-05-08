namespace UnityProjectAirSim.Weather
{
    public enum WeatherParameter
    {
        Enabled = 0,
        Rain = 1,
        RoadWetness = 2,
        Snow = 3,
        RoadSnow = 4,
        MapleLeaf = 5,
        RoadLeaf = 6,
        Dust = 7,
        Fog = 8,

        // Last
        Count,
    };

    /// <summary>
    /// Stores a set of float weather settings and applies them to instances of weather effects in the scene.
    /// </summary>
    public class WeatherParamScalarCollection
    {
        private float[] values = new float[(int)WeatherParameter.Count];

        public float this[WeatherParameter index]
        {
            get => values[(int)index];
            set => values[(int)index] = value;
        }
    }
}
