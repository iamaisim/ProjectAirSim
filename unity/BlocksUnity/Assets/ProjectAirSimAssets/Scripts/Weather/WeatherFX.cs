using UnityProjectAirSim.World;
using UnityEngine;

namespace UnityProjectAirSim.Weather
{
    /// <summary>
    /// Renders an instance of weather visual effects.
    /// </summary>
    public class WeatherFX : MonoBehaviour
    {
        public WeatherParamScalarCollection ParamScalars { get; private set; } = new WeatherParamScalarCollection();

        public bool IsEnabled { get; set; }

        // This will load Particles/P_Weather_SnowFX.prefab automatically
        [SerializeField]
        private ParticleSystem snowParticleSystem = default;

        private float snowParticleSystemMaximumRate;

        private void Start() {
            snowParticleSystemMaximumRate = snowParticleSystem.emission.rateOverTime.constant;
        }

        private void Update()
        {
            if (!IsEnabled)
                return;

            ParticleSystem.EmissionModule emission = snowParticleSystem.emission;
            emission.rateOverTime = ParamScalars[WeatherParameter.Snow] * snowParticleSystemMaximumRate;
        }

        public void Reset()
        {
            IsEnabled = false;
            gameObject.SetActive(false);
            ParamScalars = new WeatherParamScalarCollection();
        }
    }
}
